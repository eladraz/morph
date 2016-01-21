/*
 * Copyright (c) 2008-2016, Integrity Project Ltd. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * Redistributions of source code must retain the above copyright notice,
 * this list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice,
 * this list of conditions and the following disclaimer in the documentation
 * and/or other materials provided with the distribution.
 *
 * Neither the name of the Integrity Project nor the names of its contributors
 * may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE
 */

#ifndef __TBA_CLR_EXECUTER_SCANNINGALGORITHMINTERFACE_H
#define __TBA_CLR_EXECUTER_SCANNINGALGORITHMINTERFACE_H

/*
 * ScanningAlgorithmInterface.h
 *
 * Tells the runtime engine which methods should be compiled.
 *
 * Author: Elad Raz <e@eladraz.com>
 */
#include "xStl/types.h"
#include "xStl/os/event.h"
#include "data/ElementType.h"
#include "format/coreHeadersTypes.h"

/*
 * Tells the runtime engine which methods should be compiled.
 *
 * Usage example:
 *    class MyRuntimeAlgorithm : public ScanningAlgorithmInterface,
 *                               public CompilerNotifierInterface {
 *    public:
 *          virtual bool getNextMethod(nextMethodId, nextMethodApartmentId)
 *          {
 *              // Check queue of stand by methods, high-priority
 *
 *              // Check for incoming predicatable execution
 *
 *              // Check for memory-space for entire application compilations
 *          };
 *
 *         virtual void onMethodCompiled(const SecondPassBinaryPtr&)
 *         {
 *              // Check dependency for new methods
 *              if (methodsInQueue())
 *                  pulseEvent(); // Wakeup threads
 *              else
 *              {
 *                  terminate = true;
 *                  pulseEvent();
 *              }
 *         };
 *    };
 *
 *
 * NOTE: This class MUST be thread safe
 */
class ScanningAlgorithmInterface {
public:
    // You can inherit from me
    virtual ~ScanningAlgorithmInterface();

    /*
     * Return the next method (inside an apartment) that should be compiled.
     *
     * Return false if there is no more methods and the compiler thread should
     * go to sleep/terminate. In order to distinguish between these two cases
     * see 'shouldExit()'
     */
    virtual bool getNextMethod(TokenIndex& nextMethod) = 0;

    /*
     * Return true if the application completed it's execution and the threads
     * should terminate thier jobs.
     */
    virtual bool shouldExit() const = 0;

    /*
     * Return the waiting (In case there is no more methods to be compiled).
     *
     * The event is set if the queue is not empty.
     *
     * TODO! This platfrom might be changed into notifications...
     */
    const cEvent& getSleepEvent() const;

protected:
    /*
     * Should be called by the implementation in order to wake all pending
     * threads
     */
    void notify();

    // The waiting event
    cEvent m_event;
};

#endif // __TBA_CLR_EXECUTER_SCANNINGALGORITHMINTERFACE_H
