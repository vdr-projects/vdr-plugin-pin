/*
 * A plugin for the Video Disk Recorder
 *
 * See the README file for copyright information and how to reach the author.
 *
 * File: pin.h
 *
 */

#ifndef __PIN_H__
#define __PIN_H__

//***************************************************************************
// Includes
//***************************************************************************

#include <vdr/status.h>
#include <vdr/plugin.h>

#include "locks.h"
#include "def.h"
#include "talk.h"

//***************************************************************************
// Constants
//***************************************************************************

static const char *VERSION        = "0.1.17";
static const char *DESCRIPTION    = "Childlock plugin";
static const char *MAINMENUENTRY  = tr("Childlock");

#define PROTECTION_FILE "protection.fsk"
// #define __DEBUG__

//***************************************************************************
// Pin Service
//***************************************************************************

class PinService
{
   public:

      enum Size
      {
         sizePinCode = 5
      };

      enum ListType
      {
         ltUnknown = na,
         ltChannels,
         ltBroadcasts,
         ltPlugins,
         ltMenuItems
      };

      class Translation : public cListObject
      {
         public:

            char in[200+TB];
            char out[200+TB];
      };
};

class Translations : public cList<PinService::Translation>, PinService
{
   public:

      void append(const char* in, const char* out);
      const char* lookup(const char* in);

};

//***************************************************************************
// Message Reciver
//***************************************************************************

class MessageReceiver : public cThread
{
   public:

      MessageReceiver();
      ~MessageReceiver();

      int StartReceiver();
      int StopReceiver();

   protected:

      void Action();
      int wait();

      // data

      Talk* talk;
      int pid;
      bool active;
};

//***************************************************************************
// Pin Status Monitor
//***************************************************************************

class cPinStatusMonitor : public cStatus
{
   protected:

      // interface

      virtual void ChannelSwitch(const cDevice *Device, int ChannelNumber, bool LiveView);
      virtual bool ChannelProtected(const cDevice *Device, const cChannel* Channel);
      virtual bool ReplayProtected(const cRecording* Recording, const char* Name,
                                   const char* Base, bool isDirectory, int menuView = false);
      virtual void RecordingFile(const char* FileName);
      virtual void TimerCreation(cTimer* Timer, const cEvent *Event);
      virtual bool PluginProtected(cPlugin* Plugin, int menuView = false);
      virtual bool MenuItemProtected(const char* Name, int menuView = false);

#if defined (APIVERSNUM) && (APIVERSNUM >= 20301)
      virtual void UserAction(const eKeys key);
#else
      virtual void UserAction(const eKeys key, const cOsdObject* Interact);
#endif

      // internal stuff

      const cEvent* GetEventOf(const cSchedules* schedules, const cChannel* Channel);

      Translations translations;
};

//***************************************************************************
// Pin
//***************************************************************************

class cPin : public cOsdObject, public PinService
{
   public:

      cPin(void);
      virtual ~cPin();
      virtual void Show(void);
      virtual eOSState ProcessKey(eKeys Key);
      void clear();

   private:

      cOsd* osd;
      int textWidth;
      const char* pinTxt;
      char code[sizePinCode+TB];
      cSkinDisplayMessage* display;
      int pinSetup;
};

//***************************************************************************
// Pin Plugin
//***************************************************************************

class cPinPlugin : public cPlugin, public PinService
{
  public:

      enum AutoProtectionMode
      {
         apmUnknown = na,

         apmAlways,
         apmIntelligent,
         apmNever,

         apmCount
      };

      cPinPlugin(void);
      virtual ~cPinPlugin();

      const char* CommandLineHelp(void)        { return 0; }
      bool ProcessArgs(int argc, char *argv[]) { return true; }
      bool Start(void);
      void Stop(void);
      void Housekeeping(void)                  {}

      virtual cMenuSetupPage *SetupMenu(void);
      virtual bool SetupParse(const char *Name, const char *Value);
      const char* Version(void)                { return VERSION; }
      const char* Description(void)            { return trNOOP(DESCRIPTION); }
      virtual bool Initialize(void);
      virtual const char* MainMenuEntry(void)  { return tr(MAINMENUENTRY); }
      virtual cOsdObject* MainMenuAction(void);

      void addChannel();
      void delChannel();
      int channelProtected(const char* name, long startTime = 0);
      int pluginProtected(const char* name);
      int menuItemProtected(const char* text);
      int menuItemAppend(const char* text);
      int broadcastProtected(const char* title);
      void checkActivity();
      int initPluginList();
      void StorePin();

      static cPinPlugin* getObject()            { return object; }

      // static config items

      static char pinCode[sizePinCode+TB];
      static bool skipChannelSilent;
      static bool hideProtectedMenus;
      static bool hideProtectedPlugins;
      static bool hideProtectedRecordings;
      static int pinResetTime;
      static bool autoMenuOpen;
      static int autoProtectionMode;

      // other static stuff

      static const char* autoProtectionModes[apmCount+1];

  protected:

      // data

      cLockItems lockedChannels;
      cLockedBroadcasts lockedBroadcasts;
      cLockItems lockedPlugins;
      cLockItems lockedMenuItems;
      time_t lastAction;
      cPinStatusMonitor* statusMonitor;
      MessageReceiver* receiver;

      static cPinPlugin* object;    // the object
};

//***************************************************************************
#endif // __PIN_H__
