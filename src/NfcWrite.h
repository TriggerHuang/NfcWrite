//============================================================================
// Copyright [2013] <NFC>
// Name        : NfcWrite.h
// Author      : TriggerHuang
// Description : Example Application of NFC Project
//============================================================================


#pragma once
#include <gaia/core/Page.h>

namespace gaia {
    namespace core {
        class Persistence;
    }
    namespace ui {
        class LinearController;
        class TextWidget;
    }
}

class NfcWrite: public gaia::core::Page {
  public:
    NfcWrite();
    ~NfcWrite();

  protected:
    virtual void onInit(gaia::core::Persistence* const p);
    virtual void onTear();

  private:
    gaia::ui::LinearController* mpCtrl;
    gaia::ui::TextWidget* mpText;
};

