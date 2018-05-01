/*=========================================================================

  Program:   ParaViewGEOS
  Module:    pvgLoadDataReaction.cxx

  Copyright (c) 2018, Kitware Inc.
  All rights reserved.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHORS OR
CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

========================================================================*/
#include "pvgLoadDataReaction.h"

// ParaView includes
#include <pqActiveObjects.h>
#include <pqDeleteReaction.h>
#include <pqObjectBuilder.h>
#include <pqPipelineSource.h>
#include <pqUndoStack.h>
#include <vtkDataObjectAlgorithm.h>
#include <vtkPVDataInformation.h>
#include <vtkSMSourceProxy.h>
#include <vtkTrivialProducer.h>

// VTK includes
#include <vtkCompositeDataGeometryFilter.h>
#include <vtkGDALVectorReader.h>
#include <vtkGeoProjection.h>
#include <vtkGeoTransform.h>
#include <vtkImageData.h>
#include <vtkPointSet.h>
#include <vtkRasterReprojectionFilter.h>
#include <vtkSMIntVectorProperty.h>
#include <vtkTransformFilter.h>
#include <vtkTrivialProducer.h>
#include <vtkUniformGrid.h>

// STL includes
#include <regex>
#include <string>

// Qt includes
#include <QDebug>
#include <QSet>
#include <QString>

//-----------------------------------------------------------------------------
pvgLoadDataReaction::pvgLoadDataReaction(QAction* parentObject)
  : Superclass(parentObject)
{
}

//-----------------------------------------------------------------------------
void pvgLoadDataReaction::onTriggered()
{
  QList<pqPipelineSource*> sources = pqLoadDataReaction::loadData();
  pqPipelineSource* source;
  foreach (source, sources)
  {
    emit this->loadedData(source);
    pvgLoadDataReaction::createTrivialProducer(source);

    // Now that the trivial producer is created, remove the data loaded source
    QSet<pqPipelineSource*> scs;
    scs.insert(source);
    pqDeleteReaction::deleteSources(scs);
  }
}

//-----------------------------------------------------------------------------
void pvgLoadDataReaction::createTrivialProducer(pqPipelineSource* source)
{
  if (!source)
  {
    return;
  }
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  vtkSMSourceProxy* p = source->getSourceProxy();
  p->UpdatePipeline();
  vtkAlgorithm* d =
    vtkAlgorithm::SafeDownCast(source->getProxy()->GetClientSideObject());

  vtkSmartPointer<vtkDataObject> dobj;
  switch (p->GetDataInformation()->GetDataSetType())
  {
    case VTK_IMAGE_DATA:
    case VTK_UNIFORM_GRID:
    {
      vtkNew<vtkRasterReprojectionFilter> rrf;
      rrf->SetInputConnection(d->GetOutputPort());
      rrf->SetOutputProjection("EPSG:4326");
      rrf->Update();
      dobj = rrf->GetOutput();
      break;
    }
    default:
    {
      vtkNew<vtkCompositeDataGeometryFilter> cgf;
      cgf->SetInputConnection(d->GetOutputPort());
      vtkNew<vtkGeoProjection> gcs;
      gcs->SetName("latlong");
      if (strcmp(p->GetXMLName(), "GDALVectorReader") == 0)
      {
        vtkGDALVectorReader* r = vtkGDALVectorReader::SafeDownCast(d);
        gcs->SetPROJ4String(r->GetLayerProjectionAsProj4(0));
      }
      vtkNew<vtkGeoProjection> ocs;
      ocs->SetName("latlong");
      vtkNew<vtkGeoTransform> gt;
      gt->SetSourceProjection(gcs);
      gt->SetDestinationProjection(ocs);
      vtkNew<vtkTransformFilter> tf;
      tf->SetTransform(gt);
      tf->SetInputConnection(cgf->GetOutputPort());
      tf->Update();
      dobj = tf->GetOutput();
      break;
    }
  }

  pqServer* server = pqActiveObjects::instance().activeServer();
  if (!server)
  {
    qCritical() << "Cannot create reader without an active server.";
    return;
  }

  QString xmlname("TrivialProducer");
  QString xmlgroup("sources");
  BEGIN_UNDO_SET(QString("Create '%1'").arg(xmlname));
  pqPipelineSource* s = builder->createSource(xmlgroup, xmlname, server);
  s->rename(source->getSMName());
  END_UNDO_SET();
  vtkTrivialProducer* tp =
    vtkTrivialProducer::SafeDownCast(s->getProxy()->GetClientSideObject());
  tp->SetOutput(dobj);
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
