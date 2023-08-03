#pragma once

#include "../common.hpp"

namespace pimoroni {

  class DualSwitchedModule : public YukonModule {
  public:
    static const std::string NAME;

    static bool is_module(uint adc_level, bool slow1, bool slow2, bool slow3);

    virtual std::string name() {
      return DualSwitchedModule::NAME;
    }

    TYPE_FUNCTION(DualSwitchedModule)
  };

}
