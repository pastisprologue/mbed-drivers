/* Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#ifndef TRIGGEREDTIMEOUT_H
#define TRIGGEREDTIMEOUT_H

#include "platform/platform.h"
#include "platform/Callback.h"

#if DEVICE_TRIGGEREDTIMEOUT
#include "hal/triggeredtimeout_api.h"
#include "platform/critical.h"

namespace mbed {
/** \addtogroup drivers */
/** @{*/

/** An input that counts rising edges automatically using a timer.
 * 
 * 
 * Example
 * @code
 * #include "mbed.h"
 * #include "TriggeredTimeout.h"
 *
 * TriggeredTimeout triggeredTimeout(PA_15);
 * 
 * void delayedISR() {
 *		//delayedISR
 * }
 *
 * int main() {
 *		triggeredTimeout.rise(&delayedISR, 5.0);
 *		while(1) {
 *			//Loop forever
 *		}
 * }
 * @endcode
 */
class TriggeredTimeout {

public:

    TriggeredTimeout(PinName pin) {
        core_util_critical_section_enter();
        triggeredtimeout_init(&_tt, pin, &TriggeredTimeout::_irq_handler, (uint32_t)this);
        core_util_critical_section_exit();
    }

    void attach(Callback<void()> func)
    {
        if(func) {
            _function.attach(func);
            trigger_set_irq(&_tt, interval);
        } else {
            _function.attach(donothing);
            trigger_set_irq(&_tt, interval);
        }
    }

    static void _irq_handler(uint32_t id) {
        TriggeredTimeout *handler = (TriggeredTimeout*)id;
        hanlder->_function.call();
    }

    void enable_irq() {
        core_util_critical_section_enter();
        trigger_irq_enable(&_tt);
        core_util_critical_section_exit();
    } 

    void disable_irq() {
        core_util_critical_section_enter();
        trigger_irq_disable(&_tt);
        core_util_critical_section_exit();
    }

protected:
    triggeredtimeout_t _tt;

    Callback<void()> _function;
};

} // namespace mbed

#endif

#endif

/** @}*/
