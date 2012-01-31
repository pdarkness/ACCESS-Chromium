// Copyright (c) 2011 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#import "chrome/browser/ui/cocoa/applescript/bookmark_applescript_utils_unittest.h"

#include "chrome/browser/bookmarks/bookmark_model.h"

@implementation FakeAppDelegate

@synthesize test = test_;

- (Profile*)lastProfile {
  if (!test_)
    return NULL;
  return test_->profile();
}
@end

// Represents the current fake command that is executing.
static FakeScriptCommand* kFakeCurrentCommand;

@implementation FakeScriptCommand

- (id)init {
  if ((self = [super init])) {
    originalMethod_ = class_getClassMethod([NSScriptCommand class],
                                           @selector(currentCommand));
    alternateMethod_ = class_getClassMethod([self class],
                                            @selector(currentCommand));
    method_exchangeImplementations(originalMethod_, alternateMethod_);
    kFakeCurrentCommand = self;
  }
  return self;
}

+ (NSScriptCommand*)currentCommand {
  return kFakeCurrentCommand;
}

- (void)dealloc {
  method_exchangeImplementations(originalMethod_, alternateMethod_);
  kFakeCurrentCommand = nil;
  [super dealloc];
}

@end

BookmarkAppleScriptTest::BookmarkAppleScriptTest() {
}

BookmarkAppleScriptTest::~BookmarkAppleScriptTest() {
  [NSApp setDelegate:nil];
}

void BookmarkAppleScriptTest::SetUp() {
  CocoaProfileTest::SetUp();
  ASSERT_TRUE(profile());

  appDelegate_.reset([[FakeAppDelegate alloc] init]);
  [appDelegate_.get() setTest:this];
  DCHECK([NSApp delegate] == nil);
  [NSApp setDelegate:appDelegate_];
  const BookmarkNode* root = model().bookmark_bar_node();
  const std::string modelString("a f1:[ b d c ] d f2:[ e f g ] h ");
  model_test_utils::AddNodesFromModelString(model(), root, modelString);
  bookmarkBar_.reset([[BookmarkFolderAppleScript alloc]
      initWithBookmarkNode:model().bookmark_bar_node()]);
}

BookmarkModel& BookmarkAppleScriptTest::model() {
  return *profile()->GetBookmarkModel();
}