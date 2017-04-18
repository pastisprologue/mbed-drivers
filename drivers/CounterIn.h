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
#ifndef COUNTERIN_H
#define COUNTERIN_H

#include "platform/platform.h"

#if DEVICE_COUNTERIN
#include "hal/counterin_api.h"
#include "platform/critical.h"

namespace mbed {
/** \addtogroup drivers */
/** @{*/

/** An input that counts rising edges automatically using a timer.
 * 
 * Example
 * @code
 * #include "mbed.h"
 * #include "CounterIn.h"
 *
 * CounterIn (PA_15);
 *
 * int main() {
 *		counter.start();
 *		while(1) {
 *			printf("Count: %d\r\n", counter.read() );
 *		}
 * }
 * @endcode
 *
 */
class CounterIn {

public:

	/** Initializes a HW timer to accept external clock source
	 *
	 * @param pin Pin to connect an external clock source (pulse train)
	 */
    CounterIn(PinName pin) {
        core_util_critical_section_enter();
        counterin_init(&_counter, pin);
        core_util_critical_section_exit();
    }

	/** Read the current count of the HW timer
	 *
	 * @returns
	 *	An unsigned integer, with size depending on whether a 16-bit or 32-bit timer
	 *	was used
	 */
    uint32_t read() {
        core_util_critical_section_enter();
        uint32_t val = counterin_read(&_counter);
        core_util_critical_section_exit();
        return val;
    }

	void start() {
		core_util_critical_section_enter();
		counterin_start(&_counter);
		core_util_critical_section_exit();
	}

	void reset() {
		core_util_critical_section_enter();
		counterin_reset(&_counter);
		core_util_critical_section_exit();
	}

	void stop() {
		core_util_critical_section_enter();
		counterin_stop(&_counter);
		core_util_critical_section_exit();	
	}


    /** An operator shorthand for read()
     */
    operator uint32_t() {
        // Underlying read is thread safe
        return read();
    }
protected:
    counterin_t _counter;
};

} // namespace mbed

#endif

#endif

/** @}*/
