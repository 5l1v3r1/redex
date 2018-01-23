/**
 * Copyright (c) 2016-present, Facebook, Inc.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree. An additional grant
 * of patent rights can be found in the PATENTS file in the same directory.
 */

#pragma once

#include <atomic>
#include <string>

class ReferencedState {
 private:
  bool m_bytype{false};
  bool m_bystring{false};
  // m_computed is a "clear-only" flag; If one of the reflects is non-computed,
  // all subsequents should be non-computed. Reflect marking which is computed
  // from code means that it can/should be recomputed periodically when doing
  // optimizations. For instance, deleting a method with a reflection target
  // will then allow that reflection target to be re-evaluated.
  bool m_computed{true};

  // ProGuard keep settings
  //
  // Specify classes and class members that are entry-points.
  bool m_keep{false};
  // assumenosideeffects allows certain methods to be removed.
  bool m_assumenosideeffects{false};
  // Does this class have a blanket "-keepnames class *" applied to it?
  // "-keepnames" is synonym with "-keep,allowshrinking".
  bool m_blanket_keepnames{false};
  // If m_whyareyoukeeping is true then report debugging information
  // about why this class or member is being kept.
  bool m_whyareyoukeeping{false};

  // For keep modifiers: -keep,allowshrinking and -keep,allowobfuscation.
  //
  // Instead of m_allowshrinking and m_allowobfuscation, we need to have
  // set/unset pairs for easier parallelization. The unset has a high priority.
  // See the comments in apply_keep_modifiers.
  bool m_set_allowshrinking{false};
  bool m_unset_allowshrinking{false};
  bool m_set_allowobfuscation{false};
  bool m_unset_allowobfuscation{false};

  bool m_keep_name{false};

  // The number of keep rules that touch this class.
  std::atomic<unsigned int> m_keep_count{0};

 public:
  ReferencedState() = default;

  // std::atomic requires an explicitly user-defined assignment operator.
  ReferencedState& operator=(const ReferencedState& other) {
    if (this != &other) {
      this->m_bytype = other.m_bytype;
      this->m_bystring = other.m_bystring;
      this->m_computed = other.m_computed;
      
      this->m_keep = other.m_keep;
      this->m_assumenosideeffects = other.m_assumenosideeffects;
      this->m_blanket_keepnames = other.m_blanket_keepnames;
      this->m_whyareyoukeeping = other.m_whyareyoukeeping;
      
      this->m_set_allowshrinking = other.m_set_allowshrinking;
      this->m_unset_allowshrinking = other.m_unset_allowshrinking;
      this->m_set_allowobfuscation = other.m_set_allowobfuscation;
      this->m_unset_allowobfuscation = other.m_unset_allowobfuscation;
      
      this->m_keep_name = other.m_keep_name;
      
      this->m_keep_count = other.m_keep_count.load();
    }
    return *this;
  }

  std::string str() const;

  bool can_delete() const { return !m_bytype && (!m_keep || allowshrinking()); }
  bool can_rename() const {
    return !m_keep_name && !m_bystring && (!m_keep || allowobfuscation()) &&
           !allowshrinking();
  }

  // ProGuard keep options
  bool keep() const { return m_keep; }

  // ProGaurd keep option modifiers
  bool allowshrinking() const {
    return !m_unset_allowshrinking && m_set_allowshrinking;
  }
  bool allowobfuscation() const {
    return !m_unset_allowobfuscation && m_set_allowobfuscation;
  }
  bool assumenosideeffects() const { return m_assumenosideeffects; }

  bool is_blanket_names_kept() const {
    return m_blanket_keepnames && m_keep_count == 1;
  }

  bool report_whyareyoukeeping() const { return m_whyareyoukeeping; }

  // For example, a classname in a layout, e.g. <com.facebook.MyCustomView />
  // is a ref_by_string with from_code = false
  //
  // Class c = Class.forName("com.facebook.FooBar");
  // is a ref_by_string with from_code = true
  void ref_by_string(bool from_code) {
    m_bytype = m_bystring = true;
    m_computed = m_computed && from_code;
  }
  bool is_referenced_by_string() const { return m_bystring; }

  // A direct reference from code (not reflection)
  void ref_by_type() { m_bytype = true; }
  bool is_referenced_by_type() const { return m_bytype; }

  // Called before recompute
  void clear_if_compute() {
    if (m_computed) {
      m_bytype = m_bystring = false;
    }
  }

  // ProGuard keep information.
  void set_keep() { m_keep = true; }

  void set_keep_name() { m_keep_name = true; }

  void set_allowshrinking() { m_set_allowshrinking = true; }
  void unset_allowshrinking() { m_unset_allowshrinking = true; }

  void set_allowobfuscation() { m_set_allowobfuscation = true; }
  void unset_allowobfuscation() { m_unset_allowobfuscation = true; }

  void set_assumenosideeffects() { m_assumenosideeffects = true; }

  void set_blanket_keepnames() { m_blanket_keepnames = true; }

  void increment_keep_count() { m_keep_count++; }

  void set_whyareyoukeeping() { m_whyareyoukeeping = true; }
};
