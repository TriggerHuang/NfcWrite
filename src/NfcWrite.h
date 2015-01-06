//============================================================================
// Copyright [2013] <NFC>
// Name        : NfcWrite.h
// Author      : TriggerHuang
// Description : Example Application of NFC Project
//============================================================================


#pragma once
#include <gaia/core/Page.h>

namespace gaia {
    namespace connectivity {
        class NfcManager;
        class NdefMessage;
        class NdefRecord;
        class CreateNdefMessageCallback;
        class Tag;
        class Ndef;
    }  // connectivity

    namespace core {
        class OnClickListener;
        class Persistence;
        class Scene;
        class widget;
        class Context;
        class Handler;
        class Message;
        class Subscriber;
        class DemandFilter;
        class Demand;
        class Parcel;
        class PendingDemand;
    }  // core

    namespace ui {
        class Button;
        class EditWidget;
        class LinearController;
        class TextWidget;
        class ListWidget;
        class ScrollWidget;
    }  // ui
    namespace base {
        class String;
        template<typename T> class Vector;
        template <class TYPE> class Vector2D;
        class ByteArray;
    }  // base

    namespace net {
        class Uri;
    }

    namespace locale {
        class Locale;
    }
}  // gaia


class NfcWriteBtnOnClickListener;
class NfcWriteHandler;

class NfcWrite: public gaia::core::Page {
    friend class NfcWriteBtnOnClickListener;
    friend class NfcWriteHandler;
    class MyBroadcastReceiver;

  public:
    enum ButtonID {
           BTN_WRITE = 0,
           BTN_CLEAR = 1,
           BTN_MAX = 2,
       };

    NfcWrite();
    ~NfcWrite();

  public:
    virtual void onInit(gaia::core::Persistence* const p);
    virtual void onTear();
    //virtual void onNewDemand(gaia::core::Demand* const demand);
    virtual void onResume();
    virtual void onRestart();
    virtual void onPause();

    // 这个virtual必须加，否则会提示没有声明
    virtual void onNewDemand(gaia::core::Demand* const demand);

    gaia::base::String NfcWrite::bytesToString(const gaia::base::ByteArray& bs);
    void showMessage(const gaia::base::String& msg);
    void showWarning(const gaia::base::String& msg);
    void WriteTag(const gaia::core::Demand& demand);



  private:
  private:
    gaia::ui::LinearController* mpCtrl;

    gaia::core::Context* mpContext;
    gaia::ui::TextWidget* mpTextNotice;
    gaia::ui::TextWidget* mpTextSpace;
    gaia::ui::TextWidget* mpTextWarning;
    gaia::ui::ScrollWidget* mpScrollView;
    gaia::ui::EditWidget* mpWriteContent;
    bool mNfcEnabled;
    NfcWriteBtnOnClickListener *mpNfcWriteBtnOnClickListener;
    NfcWriteHandler *mpHandler;
    gaia::ui::Button* mpBtn_Write;
    gaia::ui::Button* mpBtn_Clear;
    gaia::ui::Button* mpBtn_space;


    gaia::connectivity::NfcAdapter* mpNfcAdapter;
    gaia::core::PendingDemand* mpPendingDemand;
    gaia::base::Vector<gaia::core::DemandFilter>* mpDemandFilters;
    gaia::base::String* mpTech11;
    gaia::base::Vector<gaia::base::String>* mpTechList1;
    gaia::base::Vector2D<gaia::base::String>* mpTechLists;
    gaia::core::Subscriber* mpBroadcastReceiver;
    pthread_t Rtid;
    pthread_mutex_t mutex_lock;


};

class NfcWriteBtnOnClickListener: public gaia::core::OnClickListener {
        private:
                NfcWrite* mpNfcWrite;
        public:
                NfcWriteBtnOnClickListener(NfcWrite* p);
                virtual ~NfcWriteBtnOnClickListener();
                void onClick(gaia::core::Widget* v); // overlaod

};

class NfcWriteHandler: public gaia::core::Handler {
        public:
				NfcWriteHandler(NfcWrite* p);
                void handleMessage(const gaia::core::Message& msg);
        private:
                NfcWrite* mpNfcWrite;
};

