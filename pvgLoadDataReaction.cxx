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

// Qt includes
#include <QDebug>
#include <QString>

//-----------------------------------------------------------------------------
pvgLoadDataReaction::pvgLoadDataReaction(QAction* parentObject)
  : Superclass(parentObject)
{
}

//-----------------------------------------------------------------------------
void pvgLoadDataReaction::onTriggered()
{
  pqApplicationCore* core = pqApplicationCore::instance();
  pqObjectBuilder* builder = core->getObjectBuilder();

  QList<pqPipelineSource*> sources = pqLoadDataReaction::loadData();
  pqPipelineSource* source;
  QString xmlname("TransformFilter");
  QString xmlgroup("filters");
  foreach (source, sources)
  {
    emit this->loadedData(source);
    source->updatePipeline();
    qDebug() << source << source->getProxy()->GetXMLName();
    BEGIN_UNDO_SET(QString("Create '%1'").arg(xmlname));
    pqPipelineSource* filter =
      builder->createFilter(xmlgroup, xmlname, source);
    END_UNDO_SET();
    filter->getProxy()->UpdateVTKObjects();
    filter->updatePipeline();
  }
}
