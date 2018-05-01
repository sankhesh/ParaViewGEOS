/*=========================================================================

  Program:   ParaViewGEOS
  Module:    pvgLoadDataReaction.h

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
#ifndef pvgLoadDataReaction_h
#define pvgLoadDataReaction_h

// ParaView includes
#include <pqLoadDataReaction.h>

class pvgLoadDataReaction : public pqLoadDataReaction
{
  Q_OBJECT
  typedef pqLoadDataReaction Superclass;

public:
  /**
   * Constructor. Parent cannot be NULL.
   */
  pvgLoadDataReaction(QAction* parent);

  /**
   * Given a source proxy for a vtkDataObjectAlgorithm (typically, the file
   * reader), apply a geo transformation and create a trivial producer for the
   * output without the pipeline.
   */
  static bool createTrivialProducer(pqPipelineSource* source);

protected:
  /**
   * Called when the action is triggered.
   */
  void onTriggered() override;
};

#endif // pvgLoadDataReaction_h
