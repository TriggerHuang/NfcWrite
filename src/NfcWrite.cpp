//============================================================================
// Copyright [2013] <NFC>
// Name        : NfcWrite.cpp
// Author      : TriggerHuang
// Description : Example Application of NFC Project
//============================================================================
#define DEBUG_LEVEL 0
#define LOG_TAG "NfcWriteSample"

#include <gaia/base/Array.h>
#include <gaia/base/ByteArray.h>
#include <gaia/base/String.h>
#include <gaia/base/StringArray.h>
#include <gaia/base/Vector.h>
#include <gaia/base/Vector2D.h>
#include <gaia/base/gloger.h>
#include <gaia/core/Context.h>
#include <gaia/core/Message.h>
#include <gaia/core/Handler.h>
#include <gaia/core/ContentResolver.h>
#include <gaia/core/Persistence.h>
#include <gaia/core/event/OnClickListener.h>
#include <gaia/core/vision/Widget.h>
#include <gaia/core/vision/Scene.h>
#include <gaia/graphics/Color.h>
#include <gaia/ui/Button.h>
#include <gaia/ui/EditWidget.h>
#include <gaia/ui/LinearController.h>
#include <gaia/ui/TextWidget.h>
#include <gaia/ui/ScrollWidget.h>

#include <gaia/connectivity/nfc/NfcAdapter.h>
#include <gaia/connectivity/nfc/NfcManager.h>
#include <gaia/connectivity/nfc/NdefRecord.h>
#include <gaia/connectivity/nfc/Ndef.h>
#include <gaia/connectivity/nfc/NdefMessage.h>
#include <gaia/connectivity/nfc/NfcEvent.h>
#include <gaia/connectivity/nfc/MifareClassic.h>
#include <gaia/connectivity/nfc/Tag.h>
#include <gaia/connectivity/nfc/CreateNdefMessageCallback.h>
#include <gaia/core/Subscriber.h>
#include <gaia/core/Demand.h>
#include <gaia/core/Parcel.h>
#include <gaia/core/DemandFilter.h>
#include <gaia/core/ComponentName.h>
#include <gaia/core/PendingDemand.h>
#include <gaia/net/Uri.h>
#include <gaia/storage/File.h>
#include <gaia/core/Parcelable.h>
#include <gaia/base/List.h>
#include <string>
#include <pthread.h>

#include "NfcWrite.h"
#include "nfcwrite/R.h"

#define safe_delete(ptr) \
    if ((ptr) != 0) \
    { \
        delete ptr; \
        ptr = 0; \
    }


using namespace gaia::base;
using namespace gaia::core;
using namespace gaia::graphics;
using namespace gaia::content;
using namespace gaia::ui;

using gaia::storage::File;
using gaia::net::Uri;
using gaia::base::String;
using gaia::base::Vector;
using gaia::base::Vector2D;
using gaia::core::Widget;
using gaia::core::Context;
using gaia::core::Demand;
using gaia::core::DemandFilter;
using gaia::core::Message;
using gaia::core::Parcel;
using gaia::core::Handler;
using gaia::base::ByteArray;


using gaia::connectivity::NfcAdapter;
using gaia::connectivity::NfcManager;
using gaia::connectivity::NdefMessage;
using gaia::connectivity::NdefRecord;
using gaia::connectivity::NfcEvent;
using gaia::connectivity::CreateNdefMessageCallback;
using gaia::connectivity::Tag;
using gaia::connectivity::Ndef;
using gaia::connectivity::MifareClassic;

class NfcWrite::MyBroadcastReceiver: public gaia::core::Subscriber {
  public:
    explicit MyBroadcastReceiver(NfcWrite* p)
        : mpNfcWrite(p) {
    }
    ~MyBroadcastReceiver() {
    }

    void onReceive(Context* context, const Demand& demand) {
    	GLOGENTRY();
        String action = demand.getAction();


        if (NfcAdapter::ACTION_TAG_DISCOVERED().equals(action)) {
        	mpNfcWrite->showMessage(String("Tag Discovered"));
        } else if (NfcAdapter::ACTION_NDEF_DISCOVERED().equals(action)) {
        	mpNfcWrite->showMessage(String("Ndef Discovered"));
        } else if (NfcAdapter::ACTION_TECH_DISCOVERED().equals(action)) {
        	mpNfcWrite->showMessage(String("Tech Discovered"));
        } else {
            GLOG(LOG_TAG, LOGINFO, "%s:%d, not match anyone", __FUNCTION__, __LINE__);
        }
    }

  private:
    NfcWrite* mpNfcWrite;
};


  NfcWrite::NfcWrite() :
    	mpCtrl(NULL),
    	mpTextNotice(NULL),
    	mpTextWarning(NULL),
    	mpScrollView(NULL),
    	mNfcEnabled(false),
    	mpNfcAdapter(NULL),
    	mpWriteContent(NULL),
    	mpNfcWriteBtnOnClickListener(NULL),
    	mpHandler(NULL),
    	mpBtn_Write(NULL),
    	mpBtn_Clear(NULL),
    	mpPendingDemand(NULL),
    	mpDemandFilters(NULL),
    	mpTech11(NULL),
    	mpTechList1(NULL),
    	mpTechLists(NULL),
    	mpBroadcastReceiver(NULL),
    	mpTextSpace(NULL),
    	mpBtn_space(NULL),
    	mpContext(NULL) {
}

  NfcWrite::~NfcWrite() {
}

  void NfcWrite::showMessage(const gaia::base::String& msg) {
        if (mpTextNotice == NULL) return;

        mpTextNotice->setText(msg);
  }

  void NfcWrite::showWarning(const gaia::base::String& msg) {
        if (mpTextWarning == NULL) return;

        mpTextWarning->setText(msg);
  }

  String NfcWrite::bytesToString(const ByteArray& bs) {
      String s = "";
      byte_t b;
      // for (byte b : bs) {
      for (uint32_t i = 0; i < bs.size(); i++) {
          b = bs[i];
          //s= s + String::format("%02X", b);
          s= s + String::format("%c", b);
      }
      return s;
  }



void NfcWrite::onInit(Persistence* const p) {

    mpContext = new gaia::core::Context(this);
    Scene* pScene = Scene::SceneFactory(this);
    mpCtrl = new LinearController(this);
    mpCtrl->setOrientation(LinearController::VERTICAL);


    mpTextNotice = new TextWidget(this);
    mpTextNotice->setTextColor(Color::BLUE);
    mpTextNotice->setTextSize(20);
    mpTextNotice->setText("Notice: ");

    mpScrollView = new ScrollWidget(this);
    mpScrollView->addWidget(mpTextNotice);
    mpCtrl->addWidget(mpScrollView);

    mpTextWarning = new TextWidget(this);
    mpTextWarning->setTextColor(Color::RED);
    mpTextWarning->setTextSize(25);
    mpTextWarning->setText("Warning: ");
    mpCtrl->addWidget(mpTextWarning);

    mpWriteContent = new EditWidget(this);
    mpWriteContent->setHint("请输入想要写入Nfc Tag的内容:");
    mpCtrl->addWidget(mpWriteContent);


    //mpBtn_Clear = new Button(this);
    //mpBtn_Clear->setText("点击清空输入内容");
    //mpBtn_Clear->setId(BTN_CLEAR);
    //mpBtn_Clear->setOnClickListener(mpNfcWriteBtnOnClickListener);
    //mpCtrl->addWidget(mpBtn_Clear);
    mpBtn_space = new Button(this);
    mpBtn_space->setEnabled(false);
    mpCtrl->addWidget(mpBtn_space);

    mpNfcWriteBtnOnClickListener = new NfcWriteBtnOnClickListener(this);
    mpHandler = new NfcWriteHandler(this);

    mpBtn_Write = new Button(this);
    mpBtn_Write->setText("点击将输入内容写入 NFC TAG");
    mpBtn_Write->setId(BTN_WRITE);
    mpBtn_Write->setMaxWidth(20);
    mpBtn_Write->setOnClickListener(mpNfcWriteBtnOnClickListener);
    mpCtrl->addWidget(mpBtn_Write);

    mpWriteContent->getTextN()->toString().string();

    mpNfcAdapter = NfcAdapter::getDefaultAdapterN(*mpContext);

    mpBroadcastReceiver = new MyBroadcastReceiver(this);
    DemandFilter filter5(NfcAdapter::ACTION_NDEF_DISCOVERED());
    mpContext->registerSubscriber(*mpBroadcastReceiver, filter5);

    DemandFilter filter6(NfcAdapter::ACTION_TECH_DISCOVERED());
    mpContext->registerSubscriber(*mpBroadcastReceiver, filter6);

    DemandFilter filter3(NfcAdapter::ACTION_TAG_DISCOVERED());
    mpContext->registerSubscriber(*mpBroadcastReceiver, filter3);

    Demand emptyDemand;
    ComponentName componentName(String("NfcWrite"), String(".NfcWrite"));
    emptyDemand.setComponent(componentName).setFlags(Demand::FLAG_PAGE_SINGLE_TOP);
    mpPendingDemand = PendingDemand::getPageN(*mpContext, emptyDemand, 0, 0);

    if (mpPendingDemand == NULL) {
    	showMessage("mpPendingDemand == NULL!");
    }


    //getClass();
    mpDemandFilters = new Vector<DemandFilter>();
    mpDemandFilters->append(filter3);
    mpDemandFilters->append(filter5);
    mpDemandFilters->append(filter6);


    mpTech11 = new String("MifareClassic");
    // emulator begin
    // mpTechLists = new Vector<Vector<String> >();
    mpTechLists = new Vector2D<String>();
    // emulator end
    mpTechList1 = new Vector<String>();
    mpTechList1->append(*mpTech11);

    // emulator begin
    // mpTechLists->append(*mpTechList1);
    mpTechLists->appendVector(0, *mpTechList1);

    pthread_mutex_init(&mutex_lock, NULL);

    pScene->attachController(mpCtrl);
    pScene->setupSceneLayout();
}

void NfcWrite::onTear() {
	  safe_delete(mpCtrl);
	    safe_delete(mpContext);
	    safe_delete(mpTextNotice);
	    safe_delete(mpTextWarning);
	    safe_delete(mpScrollView);
	    safe_delete(mpNfcAdapter);
	    safe_delete(mpWriteContent);
	    safe_delete(mpNfcWriteBtnOnClickListener);
	    safe_delete(mpHandler);
	    safe_delete(mpBtn_Write);
	    safe_delete(mpBtn_Clear);
	    safe_delete(mpNfcAdapter);
	    safe_delete(mpPendingDemand);
	    safe_delete(mpDemandFilters);
	    safe_delete(mpTech11);
	    safe_delete(mpTechList1);
	    safe_delete(mpTechLists);
	    safe_delete(mpBroadcastReceiver);
	    safe_delete(mpTextSpace);
	    safe_delete(mpBtn_space);
	    pthread_mutex_destroy(&mutex_lock);
    getSceneSingleton()->SceneDestroy();
}

void NfcWrite::onNewDemand(Demand* const demand) {

        GLOG(LOG_TAG, LOGINFO, "%s:%d, onNewDemand demand = %s", __FUNCTION__, __LINE__, demand->toString().string());
        showMessage("onNewDemand");

        // 注意这里不是 setDemand(demand); 加"*"是因为他的参数是引用,而不是指针
        setDemand(*demand);
    }

void thread_exit_handler(int sig)
{
        GLOG(LOG_TAG, LOGER, "exit the read thread\n");
        pthread_exit(0);
}

void *WriteTagThread(void *arg) {

		GLOGENTRY();
		NfcWrite* NfcWrite = arg;
        GLOG(LOG_TAG, LOGER, "entry the thread function\n");

        	/*
        //add signal
        struct sigaction actions;
        memset(&actions, 0, sizeof(actions));
        sigemptyset(&actions.sa_mask);
        actions.sa_flags = 0;
        actions.sa_handler = thread_exit_handler;
        sigaction(SIGUSR1,&actions,NULL);
        */

        {

        }



        return NULL;
}

void NfcWrite::WriteTag(const Demand& demand)
{
	GLOGENTRY();
	// Tag是通过Parcelable来进行进程间传递的，而Parcelable的载体又是demand
	// 所以第一部是从demand获取Parcelable
	Parcelable source = demand.getParcelableExtra(NfcAdapter::EXTRA_TAG());
	// 第二步是 通过Parcelable的getN<TYPE>来获取你希望获取的结构体
	Tag *tag = source.getN<Tag>();
	Ndef *ndef = Ndef::getN(*tag);
	// 创建Ndef Message, 把最后写的动作放在那个线程中去.
	String msgInfo = "NfcWrite::WriteTag > ";
	String payload="";
	payload = mpWriteContent->getTextN()->toString();
	msgInfo = msgInfo + payload.string();
	showMessage(msgInfo);
	//static NdefRecord* createMimeN(const gaia::base::String& mimeType, const gaia::base::ByteArray& mimeData);
	String minetype("text/plain");
	ByteArray mimedata("Hello, Trigger");
	NdefRecord::createMimeN(minetype, mimedata);
	//NdefMessage

	if (ndef) {
		ndef->connect();
		if (ndef->isWritable()) {
			//ndef->writeNdefMessage(); 杯具，没有这个借口
		}else
		{
			showWarning("Ndef is read-only!");
		}
	}else {
		showWarning("Ndef is NULL");
	}

	int ret = pthread_create(&Rtid, NULL, WriteTagThread, this);
}

void NfcWrite::onResume()
{
	GLOGENTRY();
	const Demand demand(getDemand());
	String action = demand.getAction();
	 if (NfcAdapter::ACTION_NDEF_DISCOVERED().equals(action)){
		 showMessage("Received ACTION_NDEF_DISCOVERED");
		 WriteTag(demand);
	 }

	/*
	if (mpNfcAdapter != NULL) {
	        mpNfcAdapter->enableForegroundDispatch(*this, *mpPendingDemand, *mpDemandFilters, *mpTechLists);
		 //mpNfcAdapter->enableForegroundDispatch(*this, *mpPendingDemand, *mpDemandFilters, 0);
	}else {
        GLOG(LOG_TAG, LOGINFO, "mpNfcAdapter == NULL");
    }*/
}

void NfcWrite::onPause() {
    GLOGENTRY();

    /*
    if (mpNfcAdapter != NULL) {
        mpNfcAdapter->disableForegroundDispatch(*this);
    } else {
        GLOG(LOG_TAG, LOGINFO, "mpNfcAdapter == NULL");
    }*/
}
void NfcWrite::onRestart()
{
}


NfcWriteBtnOnClickListener::NfcWriteBtnOnClickListener(NfcWrite* p):mpNfcWrite(p) {
    GLOGENTRY(LOG_TAG);
}

NfcWriteBtnOnClickListener::~NfcWriteBtnOnClickListener() {
	mpNfcWrite = NULL;
    GLOGENTRY(LOG_TAG);
}

void NfcWriteBtnOnClickListener::onClick(Widget* v){
    GLOGENTRY(LOG_TAG);
    //String warningInfo = "";
    String msgInfo = "";
    String msg = mpNfcWrite->mpWriteContent->getTextN()->toString();
    //gaia::core::Message m;
    switch(v->getId()) {
    case NfcWrite::BTN_WRITE:
    	// Write
    	if (msg.isEmpty()) {
    		mpNfcWrite->showWarning("请输入待写入NFC TAG的字符，比如Hello");
    	}else {
    		msgInfo = msgInfo + "将写入:" + msg.string();
    		mpNfcWrite->showMessage(msgInfo.string());
    		mpNfcWrite->showWarning("请将Ndef Tag靠近MPad右边框的中间位置");
    	}
        GLOG(LOG_TAG, LOGINFO, "BTN_WRITE");
            break;
    case NfcWrite::BTN_CLEAR:
    	mpNfcWrite->mpWriteContent->setText("");
    	//m.setData("Hello");
    	//mpNfcWrite->mpHandler->sendMessage(m);
          GLOG(LOG_TAG, LOGINFO, "BTN_CLEAR");
              break;
    default:
            GLOG(LOG_TAG,LOGER,"unknown button");
            break;
    }

}

NfcWriteHandler::NfcWriteHandler(NfcWrite* p):mpNfcWrite(p) {
        GLOG(LOG_TAG, LOGINFO, "create NfcWriteHandler\n");
}

void NfcWriteHandler::handleMessage(const gaia::core::Message& msg)
{
	 GLOGENTRY();
	/*
        char *str = msg.getData();
        mpComTest->mpReceiveData->append(gaia::base::String(str) + " ");
        pthread_mutex_unlock(&(mpComTest->mutex_lock));
        mpComTest->is_locked = false;
        */

	mpNfcWrite->mpWriteContent->setText("");
	mpNfcWrite->mpWriteContent->setHint("请输入想要写入Nfc Tag的内容:");
}



template class Export<NfcWrite, Page>;

