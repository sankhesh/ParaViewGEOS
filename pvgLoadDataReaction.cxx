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
#include <pqObjectBuilder.h>
#include <pqPipelineFilter.h>
#include <pqPipelineSource.h>
#include <pqUndoStack.h>
#include <vtkSMSessionProxyManager.h>
#include <vtkSMProxy.h>
#include <vtkSMSourceProxy.h>
#include <vtkPVDataInformation.h>
#include <vtkDataObjectAlgorithm.h>
#include <pqActiveObjects.h>
#include <vtkTrivialProducer.h>
#include <pqDeleteReaction.h>

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
  // QString xmlname("TransformFilter");
  // QString xmlgroup("filters");
  foreach (source, sources)
  {
    emit this->loadedData(source);
    source->updatePipeline();
    pvgLoadDataReaction::createTrivialProducer(source);
    // source->getSourceProxy()->UpdatePipelineInformation();

    // Now that the trivial producer is created, remove the data loaded source
    QSet<pqPipelineSource*> scs;
    scs.insert(source);
    pqDeleteReaction::deleteSources(scs);
    // filter->getProxy()->UpdateVTKObjects();
    // filter->updatePipeline();
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
  vtkDataObjectAlgorithm* d = vtkDataObjectAlgorithm::SafeDownCast(source->getProxy()->GetClientSideObject());
  qDebug() << source->getSMName() << source->getSourceProxy()->GetXMLName() << source->getSourceProxy()->GetDataInformation()->GetDataSetType() << d->GetClassName() << d->GetOutput()->GetClassName();
  switch(p->GetDataInformation()->GetDataSetType())
  {
    case VTK_IMAGE_DATA:
    case VTK_UNIFORM_GRID:
      break;
    default:
      break;
  }

  QString xmlname("TrivialProducer");
  QString xmlgroup("sources");
  BEGIN_UNDO_SET(QString("Create '%1'").arg(xmlname));
  //pqPipelineSource* filter =
  //  builder->createFilter(xmlgroup, xmlname, source);
  pqServer* server = pqActiveObjects::instance().activeServer();
  if (!server)
  {
    qCritical() << "Cannot create reader without an active server.";
    return;
  }

  pqPipelineSource* s =
    builder->createSource(xmlgroup, xmlname, server);
  s->rename(source->getSMName());
  END_UNDO_SET();
  vtkTrivialProducer* tp = vtkTrivialProducer::SafeDownCast(s->getProxy()->GetClientSideObject());
  tp->SetOutput(d->GetOutput());
  s->updatePipeline();
}

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------
