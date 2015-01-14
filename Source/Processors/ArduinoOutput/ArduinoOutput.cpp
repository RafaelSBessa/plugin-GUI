/*
    ------------------------------------------------------------------

    This file is part of the Open Ephys GUI
    Copyright (C) 2014 Open Ephys

    ------------------------------------------------------------------

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.

*/

#include "ArduinoOutput.h"

#include <stdio.h>

ArduinoOutput::ArduinoOutput()
    : GenericProcessor("Arduino Output"), state(false), outputChannel(13), inputChannel(-1)
{


    // FIXME: Remove hard-coded serial port paths. These aren't always
    // right, and in some cases (JUCE_MAC) are almost certainly wrong.
    std::cout << "Warning: using hard-coded path to Arduino." << std::endl;

    #if JUCE_LINUX
        setDevice("/dev/ttyACM0");
    #endif
    #if JUCE_WIN32
        setDevice("COM1");
    #endif
    #if JUCE_MAC
        setDevice("tty.usbmodemfd121");
    #endif

}

ArduinoOutput::~ArduinoOutput()
{

    if (arduino.isInitialized())
        arduino.disconnect();

}

AudioProcessorEditor* ArduinoOutput::createEditor()
{
    editor = new ArduinoOutputEditor(this, true);
    return editor;
}

void ArduinoOutput::setDevice(String devName)
{

    Time timer;

    arduino.connect(devName.toStdString());

    if (arduino.isArduinoReady())
    {

        uint32 currentTime = timer.getMillisecondCounter();

        arduino.sendProtocolVersionRequest();
        timer.waitForMillisecondCounter(currentTime + 2000);
        arduino.update();
        arduino.sendFirmwareVersionRequest();

        timer.waitForMillisecondCounter(currentTime + 4000);
        arduino.update();

        std::cout << "firmata v" << arduino.getMajorFirmwareVersion()
                  << "." << arduino.getMinorFirmwareVersion() << std::endl;

    }

    if (arduino.isInitialized())
    {

        std::cout << "Arduino is initialized." << std::endl;
        arduino.sendDigitalPinMode(outputChannel, ARD_OUTPUT);
    }
    else
    {
        std::cout << "Arduino is NOT initialized." << std::endl;
    }
}

void ArduinoOutput::handleEvent(int eventType, MidiMessage& event, int sampleNum)
{
    if (eventType == TTL)
    {
        const uint8* dataptr = event.getRawData();

        int eventNodeId = *(dataptr+1);
        int eventId = *(dataptr+2);
        int eventChannel = *(dataptr+3);

       // std::cout << "Received event from " << eventNodeId <<
        //          " on channel " << eventChannel <<
        //          " with value " << eventId << std::endl;

        if (inputChannel == -1 || eventChannel == inputChannel)
        {
            if (eventId == 0)
            {
                arduino.sendDigital(outputChannel, ARD_LOW);
                state = false;
            }
            else
            {
                arduino.sendDigital(outputChannel, ARD_HIGH);
                state = true;
            }
        }



        //ArduinoOutputEditor* ed = (ArduinoOutputEditor*) getEditor();
        //ed->receivedEvent();
    }

}

void ArduinoOutput::setParameter(int parameterIndex, float newValue)
{
    editor->updateParameterButtons(parameterIndex);

}

void ArduinoOutput::setOutputChannel(int chan)
{
    outputChannel = chan;
}

void ArduinoOutput::setInputChannel(int chan)
{
    inputChannel = chan;
}

bool ArduinoOutput::enable()
{

}

bool ArduinoOutput::disable()
{
    arduino.sendDigital(outputChannel, ARD_LOW);
}

void ArduinoOutput::process(AudioSampleBuffer& buffer,
                            MidiBuffer& events)
{


    checkForEvents(events);


}
