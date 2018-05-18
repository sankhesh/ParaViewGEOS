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
    if (pvgLoadDataReaction::createTrivialProducer(source))
    {
      // Now that the trivial producer is created,
      // remove the original source
      QSet<pqPipelineSource*> scs;
      scs.insert(source);
      pqDeleteReaction::deleteSources(scs);
    }
  }
}

//-----------------------------------------------------------------------------
bool pvgLoadDataReaction::createTrivialProducer(pqPipelineSource* source)
{
  if (!source)
  {
    return false;
  }
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  vtkSMSourceProxy* p = source->getSourceProxy();
  p->UpdatePipeline();
  vtkAlgorithm* d = vtkAlgorithm::SafeDownCast(p->GetClientSideObject());

  vtkSmartPointer<vtkDataObject> dobj;
  bool create = false;
  switch (p->GetDataInformation()->GetDataSetType())
  {
    case VTK_IMAGE_DATA:
    case VTK_UNIFORM_GRID:
    {
      if (strcmp(p->GetXMLName(), "GDALRasterReader") == 0)
      {
        // The incoming dataset will have projection information only if it was
        // read using the vtkGDALRasterReader
        vtkNew<vtkRasterReprojectionFilter> rrf;
        rrf->SetInputConnection(d->GetOutputPort());
        rrf->SetOutputProjection("EPSG:4326");
        rrf->Update();
        dobj = rrf->GetOutput();
        create = true;
      }
      break;
    }
    default:
    {
      if (strcmp(p->GetXMLName(), "GDALVectorReader") == 0)
      {
        vtkGDALVectorReader* r = vtkGDALVectorReader::SafeDownCast(d);
        const char* proj = r->GetLayerProjectionAsProj4(0);
        if (proj && strlen(proj))
        {
          // If the input has a projection defined, transform it.
          // Else, pass it along as is.
          vtkNew<vtkCompositeDataGeometryFilter> cgf;
          cgf->SetInputConnection(d->GetOutputPort());
          vtkNew<vtkGeoProjection> ics;
          ics->SetPROJ4String(proj);
          vtkNew<vtkGeoProjection> ocs;
          ocs->SetName("latlong");
          vtkNew<vtkGeoTransform> gt;
          gt->SetSourceProjection(ics);
          gt->SetDestinationProjection(ocs);
          vtkNew<vtkTransformFilter> tf;
          tf->SetTransform(gt);
          tf->SetInputConnection(cgf->GetOutputPort());
          tf->Update();
          dobj = tf->GetOutput();
          create = true;
        }
        delete[] proj;
      }
      break;
    }
  }

  pqServer* server = pqActiveObjects::instance().activeServer();
  if (!server)
  {
    qCritical() << "Cannot create reader without an active server.";
    return false;
  }

  if (create)
  {
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
  return create;
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
