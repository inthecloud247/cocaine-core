#ifndef COCAINE_DRIVER_AUTO_TIMED_HPP
#define COCAINE_DRIVER_AUTO_TIMED_HPP

#include "cocaine/drivers/timed.hpp"

namespace cocaine { namespace engine { namespace driver {

class auto_timed_t:
    public timed_t<auto_timed_t>
{
    public:
        auto_timed_t(engine_t* engine,
                     const std::string& method, 
                     const Json::Value& args);

        // Driver interface
        virtual Json::Value info() const;

        // Timed driver interface
        ev::tstamp reschedule(ev::tstamp now);

    private:
        const ev::tstamp m_interval;
};

}}}

#endif