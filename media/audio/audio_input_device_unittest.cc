// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "base/environment.h"
#include "base/memory/scoped_ptr.h"
#include "base/win/scoped_com_initializer.h"
#include "media/audio/audio_manager.h"
#include "media/audio/audio_manager_base.h"
#if defined(OS_WIN)
#include "media/audio/win/audio_manager_win.h"
#endif
#include "testing/gtest/include/gtest/gtest.h"

using base::win::ScopedCOMInitializer;
using media::AudioDeviceNames;

// Test fixture which allows us to override the default enumeration API on
// Windows.
class AudioInputDeviceTest
    : public ::testing::Test {
 protected:
  AudioInputDeviceTest() : com_init_(ScopedCOMInitializer::kMTA) {
    audio_manager_ = AudioManager::Create();
  }

#if defined(OS_WIN)
  bool SetMMDeviceEnumeration() {
    AudioManagerWin* amw = static_cast<AudioManagerWin*>(audio_manager_.get());
    // Windows Wave is used as default if Windows XP was detected =>
    // return false since MMDevice is not supported on XP.
    if (amw->enumeration_type() == AudioManagerWin::kWaveEnumeration)
      return false;

    amw->SetEnumerationType(AudioManagerWin::kMMDeviceEnumeration);
    return true;
  }

  void SetWaveEnumeration() {
    AudioManagerWin* amw = static_cast<AudioManagerWin*>(audio_manager_.get());
    amw->SetEnumerationType(AudioManagerWin::kWaveEnumeration);
  }
#endif

  // Helper method which verifies that the device list starts with a valid
  // default record followed by non-default device names.
  static void CheckDeviceNames(const AudioDeviceNames& device_names) {
    if (!device_names.empty()) {
      AudioDeviceNames::const_iterator it = device_names.begin();

      // The first device in the list should always be the default device.
      EXPECT_EQ(std::string(AudioManagerBase::kDefaultDeviceName),
                it->device_name);
      EXPECT_EQ(std::string(AudioManagerBase::kDefaultDeviceId), it->unique_id);
      ++it;

      // Other devices should have non-empty name and id and should not contain
      // default name or id.
      while (it != device_names.end()) {
        EXPECT_FALSE(it->device_name.empty());
        EXPECT_FALSE(it->unique_id.empty());
        EXPECT_NE(std::string(AudioManagerBase::kDefaultDeviceName),
                  it->device_name);
        EXPECT_NE(std::string(AudioManagerBase::kDefaultDeviceId),
                  it->unique_id);
        ++it;
      }
    } else {
      // Log a warning so we can see the status on the build bots.  No need to
      // break the test though since this does successfully test the code and
      // some failure cases.
      LOG(WARNING) << "No input devices detected";
    }
  }

  scoped_refptr<AudioManager> audio_manager_;

  // The MMDevice API requires COM to be initialized on the current thread.
  ScopedCOMInitializer com_init_;
};

// Test that devices can be enumerated.
TEST_F(AudioInputDeviceTest, EnumerateDevices) {
  AudioDeviceNames device_names;
  audio_manager_->GetAudioInputDeviceNames(&device_names);
  CheckDeviceNames(device_names);
}

// Run additional tests for Windows since enumeration can be done using
// two different APIs. MMDevice is default for Vista and higher and Wave
// is default for XP and lower.
#if defined(OS_WIN)

// Override default enumeration API and force usage of Windows MMDevice.
// This test will only run on Windows Vista and higher.
TEST_F(AudioInputDeviceTest, EnumerateDevicesWinMMDevice) {
  AudioDeviceNames device_names;
  if (!SetMMDeviceEnumeration()) {
    // Usage of MMDevice will fail on XP and lower.
    LOG(WARNING) << "MM device enumeration is not supported.";
    return;
  }
  audio_manager_->GetAudioInputDeviceNames(&device_names);
  CheckDeviceNames(device_names);
}

// Override default enumeration API and force usage of Windows Wave.
// This test will run on Windows XP, Windows Vista and Windows 7.
TEST_F(AudioInputDeviceTest, EnumerateDevicesWinWave) {
  AudioDeviceNames device_names;
  SetWaveEnumeration();
  audio_manager_->GetAudioInputDeviceNames(&device_names);
  CheckDeviceNames(device_names);
}

#endif