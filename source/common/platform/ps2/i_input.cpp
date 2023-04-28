#include <cstdio>
#include <loadfile.h>
#include <libpad.h>
#include <kernel.h>
#include "keydef.h"
#include "cmdlib.h"
#include "m_joy.h"
#include "d_eventbase.h"

CVAR (Bool,  use_mouse,				true, CVAR_ARCHIVE|CVAR_GLOBALCONFIG)

static char padBuf[256] __attribute__((aligned(64)));
static char actAlign[6];
static int actuators;

static padButtonStatus buttons;
static u32 paddata;
static u32 old_pad = 0;
static u32 new_pad;

static const int padPort = 0;
static const int padSlot = 0;

/*
 * loadModules()
 */
static void
loadModules(void)
{
    int ret;


    ret = SifLoadModule("rom0:SIO2MAN", 0, NULL);
    if (ret < 0) {
        printf("sifLoadModule sio failed: %d\n", ret);
        SleepThread();
    }

    ret = SifLoadModule("rom0:PADMAN", 0, NULL);
    if (ret < 0) {
        printf("sifLoadModule pad failed: %d\n", ret);
        SleepThread();
    }
}

/*
 * waitPadReady()
 */
static int waitPadReady(int port, int slot)
{
    int state;
    int lastState;
    char stateString[16];

    state = padGetState(port, slot);
    lastState = -1;
    while((state != PAD_STATE_STABLE) && (state != PAD_STATE_FINDCTP1)) {
        if (state != lastState) {
            padStateInt2String(state, stateString);
            printf("Please wait, pad(%d,%d) is in state %s\n",
                       port, slot, stateString);
        }
        lastState = state;
        state=padGetState(port, slot);
    }
    // Were the pad ever 'out of sync'?
    if (lastState != -1) {
        printf("Pad OK!\n");
    }
    return 0;
}

/*
 * initializePad()
 */
static int
initializePad(int port, int slot)
{

    int ret;
    int modes;
    int i;

    waitPadReady(port, slot);

    // How many different modes can this device operate in?
    // i.e. get # entrys in the modetable
    modes = padInfoMode(port, slot, PAD_MODETABLE, -1);
    printf("The device has %d modes\n", modes);

    if (modes > 0) {
        printf("( ");
        for (i = 0; i < modes; i++) {
            printf("%d ", padInfoMode(port, slot, PAD_MODETABLE, i));
        }
        printf(")");
    }

    printf("It is currently using mode %d\n",
               padInfoMode(port, slot, PAD_MODECURID, 0));

    // If modes == 0, this is not a Dual shock controller
    // (it has no actuator engines)
    if (modes == 0) {
        printf("This is a digital controller?\n");
        return 1;
    }

    // Verify that the controller has a DUAL SHOCK mode
    i = 0;
    do {
        if (padInfoMode(port, slot, PAD_MODETABLE, i) == PAD_TYPE_DUALSHOCK)
            break;
        i++;
    } while (i < modes);
    if (i >= modes) {
        printf("This is no Dual Shock controller\n");
        return 1;
    }

    // If ExId != 0x0 => This controller has actuator engines
    // This check should always pass if the Dual Shock test above passed
    ret = padInfoMode(port, slot, PAD_MODECUREXID, 0);
    if (ret == 0) {
        printf("This is no Dual Shock controller??\n");
        return 1;
    }

    printf("Enabling dual shock functions\n");

    // When using MMODE_LOCK, user cant change mode with Select button
    padSetMainMode(port, slot, PAD_MMODE_DUALSHOCK, PAD_MMODE_LOCK);

    waitPadReady(port, slot);
    printf("infoPressMode: %d\n", padInfoPressMode(port, slot));

    waitPadReady(port, slot);
    printf("enterPressMode: %d\n", padEnterPressMode(port, slot));

    waitPadReady(port, slot);
    actuators = padInfoAct(port, slot, -1, 0);
    printf("# of actuators: %d\n",actuators);

    if (actuators != 0) {
        actAlign[0] = 0;   // Enable small engine
        actAlign[1] = 1;   // Enable big engine
        actAlign[2] = 0xff;
        actAlign[3] = 0xff;
        actAlign[4] = 0xff;
        actAlign[5] = 0xff;

        waitPadReady(port, slot);
        printf("padSetActAlign: %d\n",
                   padSetActAlign(port, slot, actAlign));
    }
    else {
        printf("Did not find any actuators.\n");
    }

    waitPadReady(port, slot);

    return 1;
}

void SetupPad()
{
	printf("SetupPad();\n");
	loadModules();
	padInit(0);

	int ret = 0;
	if((ret = padPortOpen(padPort, padSlot, padBuf)) == 0) {
		printf("padOpenPort failed: %d\n", ret);
		SleepThread();
	}

	initializePad(padPort, padSlot);
}

void I_GetEvent()
{
	int ret=padGetState(padPort, padSlot);
	while((ret != PAD_STATE_STABLE) && (ret != PAD_STATE_FINDCTP1)) {
		if(ret==PAD_STATE_DISCONN) {
			printf("Pad(%d, %d) is disconnected\n", padPort, padSlot);
		}
		ret=padGetState(padPort, padSlot);
	}

	ret = padRead(padPort, padSlot, &buttons); // port, slot, buttons

	if (ret != 0) {
		paddata = 0xffff ^ buttons.btns;

		new_pad = paddata;

		event_t ev = { EV_None };

		if((new_pad ^ old_pad) & PAD_CROSS)
		{
			ev.type = EV_GUI_Event;
			ev.subtype = ((new_pad & PAD_CROSS) ? EV_GUI_KeyDown : EV_GUI_KeyUp);
			//ev.type = (new_pad & PAD_CROSS) ? EV_KeyDown : EV_KeyUp;
			//ev.data1 = 0x13;
			ev.data1 = '\r';
		}
		if((new_pad ^ old_pad) & PAD_TRIANGLE)
		{
			ev.type = ((new_pad & PAD_TRIANGLE) ? EV_KeyDown : EV_KeyUp);
			ev.data1 = KEY_MOUSE1;
		}
		if((new_pad ^ old_pad) & PAD_UP)
		{
			ev.type = ((new_pad & PAD_UP) ? EV_KeyDown : EV_KeyUp);
			ev.data1 = 0x11; //DIK_W
			ev.data2 = 'w';
		}
		if((new_pad ^ old_pad) & PAD_DOWN)
		{
			ev.type = ((new_pad & PAD_DOWN) ? EV_KeyDown : EV_KeyUp);
			ev.data1 = 0x1F; //DIK_S
			ev.data2 = 's';
		}
		if((new_pad ^ old_pad) & PAD_LEFT)
		{
			ev.type = ((new_pad & PAD_LEFT) ? EV_KeyDown : EV_KeyUp);
			ev.data1 = 0xCB; //KEY_LEFTARROW
		}
		if((new_pad ^ old_pad) & PAD_RIGHT)
		{
			ev.type = ((new_pad & PAD_RIGHT) ? EV_KeyDown : EV_KeyUp);
			ev.data1 = 0xCD; //KEY_RIGHTARROW
		}

		old_pad = new_pad;

		if(ev.type != EV_None)
		{
			D_PostEvent(&ev);
		}
	}
}

void I_StartTic()
{
	I_GetEvent();
}

void I_StartFrame()
{
}

void I_SetMouseCapture()
{
}

void I_ReleaseMouseCapture()
{
}

void I_ShutdownInput()
{
}

void I_GetJoysticks(TArray<IJoystickConfig*>& sticks)
{
}

void I_GetAxes(float axes[NUM_JOYAXIS])
{
}

IJoystickConfig* I_UpdateDeviceList()
{
	return nullptr;
}
