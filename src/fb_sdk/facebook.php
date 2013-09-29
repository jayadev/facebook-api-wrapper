<?php
/**
 * Copyright 2011 Facebook, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License"); you may
 * not use this file except in compliance with the License. You may obtain
 * a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. See the
 * License for the specific language governing permissions and limitations
 * under the License.
 */

require_once "base_facebook.php";

/**
 * Extends the BaseFacebook class with the intent of using
 * it with cubesales mobile app, not session or storing state here
 */
class Facebook extends BaseFacebook
{

  /**
   * @see BaseFacebook::__construct in facebook.php
   */
  public function __construct($config) {
    parent::__construct($config);
  }

  /**
   * Provides the implementations of the inherited abstract
   * methods.  The implementation uses PHP sessions to maintain
   * a store for authorization codes, user ids, CSRF states, and
   * access tokens.
   */
  protected function setPersistentData($key, $value) {
  }

  protected function getPersistentData($key, $default = false) {
  }

  protected function clearPersistentData($key) {
  }

  protected function clearAllPersistentData() {
  }
}
