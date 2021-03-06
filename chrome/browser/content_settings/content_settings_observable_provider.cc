// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "chrome/browser/content_settings/content_settings_observable_provider.h"

namespace content_settings {

// ////////////////////////////////////////////////////////////////////////////
// ObservableProvider
//

ObservableProvider::ObservableProvider() {
}

ObservableProvider::~ObservableProvider() {
}

void ObservableProvider::AddObserver(Observer* observer) {
  observer_list_.AddObserver(observer);
}

void ObservableProvider::RemoveObserver(Observer* observer) {
  observer_list_.RemoveObserver(observer);
}

void ObservableProvider::RemoveAllObservers() {
  observer_list_.Clear();
}

void ObservableProvider::NotifyObservers(
    ContentSettingsPattern primary_pattern,
    ContentSettingsPattern secondary_pattern,
    ContentSettingsType content_type,
    std::string resource_identifier) {
  FOR_EACH_OBSERVER(Observer,
                    observer_list_,
                    OnContentSettingChanged(
                        primary_pattern,
                        secondary_pattern,
                        content_type,
                        resource_identifier));
}

}  // namespace content_settings
