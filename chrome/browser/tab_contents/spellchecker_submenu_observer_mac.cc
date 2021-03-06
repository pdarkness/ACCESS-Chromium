// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/tab_contents/spellchecker_submenu_observer.h"

#include "base/logging.h"
#include "chrome/app/chrome_command_ids.h"
#include "chrome/browser/prefs/pref_service.h"
#include "chrome/browser/spellchecker/spellcheck_platform_mac.h"
#include "chrome/browser/tab_contents/render_view_context_menu.h"
#include "chrome/browser/tab_contents/spelling_bubble_model.h"
#include "chrome/common/chrome_switches.h"
#include "chrome/common/pref_names.h"
#include "chrome/common/spellcheck_messages.h"
#include "content/browser/renderer_host/render_view_host.h"
#include "content/browser/renderer_host/render_widget_host_view.h"
#include "grit/generated_resources.h"
#include "ui/base/l10n/l10n_util.h"
#include "ui/base/models/simple_menu_model.h"

using content::BrowserThread;

SpellCheckerSubMenuObserver::SpellCheckerSubMenuObserver(
    RenderViewContextMenuProxy* proxy,
    ui::SimpleMenuModel::Delegate* delegate,
    int group)
    : proxy_(proxy),
      submenu_model_(delegate),
      check_spelling_while_typing_(false) {
  DCHECK(proxy_);
}

SpellCheckerSubMenuObserver::~SpellCheckerSubMenuObserver() {
}

void SpellCheckerSubMenuObserver::InitMenu(const ContextMenuParams& params) {
  DCHECK(BrowserThread::CurrentlyOn(BrowserThread::UI));

  check_spelling_while_typing_ = params.spellcheck_enabled;

  // Add an item that toggles the spelling panel.
  submenu_model_.AddCheckItem(
      IDC_SPELLPANEL_TOGGLE,
      l10n_util::GetStringUTF16(
          spellcheck_mac::SpellingPanelVisible() ?
              IDS_CONTENT_CONTEXT_HIDE_SPELLING_PANEL :
              IDS_CONTENT_CONTEXT_SHOW_SPELLING_PANEL));
  submenu_model_.AddSeparator();

  // Add a 'Check Spelling While Typing' item in the sub menu.
  submenu_model_.AddCheckItem(
      IDC_CHECK_SPELLING_OF_THIS_FIELD,
      l10n_util::GetStringUTF16(
          IDS_CONTENT_CONTEXT_CHECK_SPELLING_WHILE_TYPING));

  proxy_->AddSubMenu(
      IDC_SPELLCHECK_MENU,
      l10n_util::GetStringUTF16(IDS_CONTENT_CONTEXT_SPELLCHECK_MENU),
      &submenu_model_);
}

bool SpellCheckerSubMenuObserver::IsCommandIdSupported(int command_id) {
  switch (command_id) {
    case IDC_CONTENT_CONTEXT_LANGUAGE_SETTINGS:
      // Return false so RenderViewContextMenu can handle this item because it
      // is hard for this class to handle it.
      return false;

    case IDC_CHECK_SPELLING_OF_THIS_FIELD:
    case IDC_SPELLPANEL_TOGGLE:
    case IDC_SPELLCHECK_MENU:
    case IDC_CONTENT_CONTEXT_SPELLING_TOGGLE:
      return true;
  }

  return false;
}

bool SpellCheckerSubMenuObserver::IsCommandIdChecked(int command_id) {
  DCHECK(IsCommandIdSupported(command_id));

  // Check box for 'Check the Spelling of this field'.
  if (command_id == IDC_CHECK_SPELLING_OF_THIS_FIELD) {
    Profile* profile = proxy_->GetProfile();
    if (!profile || !profile->GetPrefs()->GetBoolean(prefs::kEnableSpellCheck))
      return false;
    return check_spelling_while_typing_;
  }

  return false;
}

bool SpellCheckerSubMenuObserver::IsCommandIdEnabled(int command_id) {
  DCHECK(IsCommandIdSupported(command_id));

  Profile* profile = proxy_->GetProfile();
  if (!profile)
    return false;

  const PrefService* pref = profile->GetPrefs();

  switch (command_id) {
    case IDC_CHECK_SPELLING_OF_THIS_FIELD:
      return pref->GetBoolean(prefs::kEnableSpellCheck);

    case IDC_SPELLPANEL_TOGGLE:
    case IDC_SPELLCHECK_MENU:
    case IDC_CONTENT_CONTEXT_SPELLING_TOGGLE:
      return true;
  }

  return false;
}

void SpellCheckerSubMenuObserver::ExecuteCommand(int command_id) {
  DCHECK(IsCommandIdSupported(command_id));

  RenderViewHost* rvh = proxy_->GetRenderViewHost();
  switch (command_id) {
    case IDC_CHECK_SPELLING_OF_THIS_FIELD:
      rvh->Send(new SpellCheckMsg_ToggleSpellCheck(rvh->routing_id()));
      break;

    case IDC_SPELLPANEL_TOGGLE:
      rvh->Send(new SpellCheckMsg_ToggleSpellPanel(
          rvh->routing_id(), spellcheck_mac::SpellingPanelVisible()));
      break;
  }
}
