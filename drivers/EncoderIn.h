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
#ifndef ENCODERIN_H
#define ENCODERIN_H

#include "platform/platform.h"

#if DEVICE_ENCODERIN

#include "hal/encoderin_api.h"
#include "platform/critical.h"

namespace mbed {

static void donothing() {}

/** A Hardware Timer implementation of an Encoder
 * 
 * Example
 * @code
 * #include "mbed.h"
 * #include "EncoderIn.h"
 *
 * EncoderIn qei(PB_4, PB_5);
 *
 * int main() {
 *		qei.start();
 *		while(1) {
 *			printf("Position: %d\r\n", qei.read() );
 *		}
 * }
 * @endcode
 */
class EncoderIn {

public:
	
	/** Create an Encoder with two Inputs
	 *
	 * @param chA Encoder Channel A Pin to connect to
	 * @param chB Encoder Channel B Pin to connect to
	 */
	EncoderIn(PinName chA, PinName chB) {
		core_util_critical_section_enter();
        encoderin_init(&_encoder, chA, chB, &EncoderIn::_irq_handler, (uint32_t)this);
        core_util_critical_section_exit();
	}

	/** Return the current Position of the encoder in ticks
	 *
	 * @returns
	 *	An integer whose size depends on the timer
	 */
	int32_t read() {
		core_util_critical_section_enter();
        int32_t val = (int16_t)encoderin_read(&_encoder);
        core_util_critical_section_exit();
        return val;
    }

	/** Starts the HW timer counting
	 */
	void start() {
		core_util_critical_section_enter();
		encoderin_start(&_encoder);
		core_util_critical_section_exit();
	}

	/** Resets the HW timer counter
	 */
	void reset() {
		core_util_critical_section_enter();
		encoderin_reset(&_encoder);
		core_util_critical_section_exit();
	}

	/** Stops the HW timer counting
	 */
	void stop() {
		core_util_critical_section_enter();
		encoderin_stop(&_encoder);
		core_util_critical_section_exit();	
	}

	/** Attach a function to be called when the Encoder has reached a certain position
	 *
	 * @param func pointer to the function to be called
	 * @param interval the location at which to trigger
	 */
    void alarm1(Callback<void()> func, uint32_t interval) {
        core_util_critical_section_enter();
        if (func) {
            _alarm1.attach(func);
            encoderin_set_irq(&_encoder, IRQ_ALARM1, interval);
        }        
        else {
            _alarm1.attach(donothing);
            encoderin_set_irq(&_encoder, IRQ_ALARM1, interval);
        }
        core_util_critical_section_exit();
    }

	/** Attach a function to be called by the Encoder has reached a certain position
	 *
	 * @param func pointer to the function to be called
	 * @param interval the location at which to trigger
	 */
    void alarm2(Callback<void()> func, uint32_t interval) {
        core_util_critical_section_enter();
        if (func) {
            _alarm2.attach(func);
            encoderin_set_irq(&_encoder, IRQ_ALARM2, interval);
        } else {
            _alarm2.attach(donothing);
            encoderin_set_irq(&_encoder, IRQ_ALARM1, interval);
        }       
        core_util_critical_section_exit();
    }

    static void _irq_handler(uint32_t id, enc_irq_event event) {
        EncoderIn *handler = (EncoderIn*)id;
        switch (event) {
            case IRQ_ALARM1: handler->_alarm1.call(); break;
            case IRQ_ALARM2: handler->_alarm2.call(); break;
        }
    }

    void enable_irq() {
        core_util_critical_section_enter();
        encoderin_irq_enable(&_encoder);
        core_util_critical_section_exit();
    } 

    void disable_irq() {
        core_util_critical_section_enter();
        encoderin_irq_disable(&_encoder);
        core_util_critical_section_exit();
    }

protected:
	encoderin_t _encoder;
    Callback<void()> _alarm1;
    Callback<void()> _alarm2;
}; //class EncoderIn

} // namespace mbed

#endif //DEVICE_ENCODERIN

#endif //ENCODERIN_H
