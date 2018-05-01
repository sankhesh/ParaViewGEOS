/*=========================================================================

  Program:   ParaViewGEOS
  Module:    pvgMainWindow.cxx

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
// ParaViewGeo includes
#include "pvgLoadDataReaction.h"
#include "pvgMainWindow.h"
#include "ui_pvgMainWindow.h"

// ParaView includes
#ifdef PARAVIEW_USE_QTHELP
#include <pqHelpReaction.h>
#endif
#include <pqParaViewBehaviors.h>
#include <pqParaViewMenuBuilders.h>
#include <pqRecentFilesMenu.h>

class pvgMainWindow::pvgInternals : public Ui::pvgMainWindow
{
};

//-----------------------------------------------------------------------------
pvgMainWindow::pvgMainWindow()
{
  this->Internals = new pvgInternals();
  this->Internals->setupUi(this);

  // Set up the dock window corners to give the vertical docks more room.
  this->setCorner(Qt::BottomLeftCorner, Qt::LeftDockWidgetArea);
  this->setCorner(Qt::BottomRightCorner, Qt::RightDockWidgetArea);

  // Enable help from the properties panel.
  QObject::connect(this->Internals->propertiesPanel,
                   SIGNAL(helpRequested(const QString&, const QString&)),
                   this,
                   SLOT(showHelpForProxy(const QString&, const QString&)));

  /// We do need the recent files menu, so set it up.
  new pqRecentFilesMenu(*this->Internals->menuRecent_Files,
                        this->Internals->menuRecent_Files);
  new pvgLoadDataReaction(this->Internals->actionOpen);

  // Populate application menus with actions.
  // pqParaViewMenuBuilders::buildFileMenu(*this->Internals->menuFile);
  pqParaViewMenuBuilders::buildEditMenu(*this->Internals->menuEdit);

  // Populate sources menu.
  pqParaViewMenuBuilders::buildSourcesMenu(*this->Internals->menuSources, this);

  // Populate filters menu.
  // pqParaViewMenuBuilders::buildFiltersMenu(*this->Internals->menuFilters,
  // this);

  // Populate Tools menu.
  pqParaViewMenuBuilders::buildToolsMenu(*this->Internals->menuTools);

  // setup the context menu for the pipeline browser.
  // pqParaViewMenuBuilders::buildPipelineBrowserContextMenu(
  //   *this->Internals->pipelineBrowser->contextMenu());

  // pqParaViewMenuBuilders::buildToolbars(*this);

  // Setup the View menu. This must be setup after all toolbars and dockwidgets
  // have been created.
  pqParaViewMenuBuilders::buildViewMenu(*this->Internals->menuView, *this);

  // Setup the menu to show macros.
  // pqParaViewMenuBuilders::buildMacrosMenu(*this->Internals->menu_Macros);

  // Setup the help menu.
  pqParaViewMenuBuilders::buildHelpMenu(*this->Internals->menuHelp);

  // Final step, define application behaviors. Since we want all ParaView
  // behaviors, we use this convenience method.
  new pqParaViewBehaviors(this, this);
}

//-----------------------------------------------------------------------------
pvgMainWindow::~pvgMainWindow()
{
  delete this->Internals;
}

//-----------------------------------------------------------------------------
void pvgMainWindow::showHelpForProxy(const QString& groupname,
                                     const QString& proxyname)
{
#ifdef PARAVIEW_USE_QTHELP
  pqHelpReaction::showProxyHelp(groupname, proxyname);
#endif
}
