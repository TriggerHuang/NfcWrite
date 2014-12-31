//============================================================================
// Copyright [2013] <NFC>
// Name        : NfcWrite.cpp
// Author      : TriggerHuang
// Description : Example Application of NFC Project
//============================================================================
#define DEBUG_LEVEL 0
#define LOG_TAG "NfcWriteSample"

#include <gaia/core/Context.h>
#include <gaia/core/Persistence.h>
#include <gaia/core/vision/Scene.h>
#include <gaia/ui/LinearController.h>
#include <gaia/ui/TextWidget.h>
#include <gaia/base/String.h>
#include "NfcWrite.h"
#include "nfcwrite/R.h"

using namespace gaia::core;
using namespace gaia::ui;
using namespace gaia::base;

  NfcWrite::NfcWrite() :
    mpCtrl(NULL),
    mpText(NULL) {
}

  NfcWrite::~NfcWrite() {
}

void NfcWrite::onInit(Persistence* const p) {
    Scene* pScene = Scene::SceneFactory(this);

    mpText = new TextWidget(this);
    mpText->setText(nfcwrite::R::string::app_name);
    mpText->setTextSize(30);

    mpCtrl = new LinearController(this);
    mpCtrl->addWidget(mpText);

    pScene->attachController(mpCtrl);
    pScene->setupSceneLayout();
}

void NfcWrite::onTear() {
    delete mpCtrl;
    delete mpText;
    getSceneSingleton()->SceneDestroy();
}

template class Export<NfcWrite, Page>;

