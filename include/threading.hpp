#ifndef YAPPI_THREADING_HPP
#define YAPPI_THREADING_HPP

#include <boost/thread.hpp>

#include "common.hpp"
#include "persistance.hpp"
#include "plugin.hpp"
#include "sockets.hpp"

namespace yappi { namespace engine { namespace detail {

// Thread facade
class thread_t:
    public helpers::birth_control_t< thread_t, helpers::limited_t<100> >
{
    public:
        thread_t(zmq::context_t& context, boost::shared_ptr<plugin::source_t> source,
            persistance::storage_t& storage, helpers::auto_uuid_t id = helpers::auto_uuid_t());
        ~thread_t();

        inline std::string id() const { return m_id.get(); }

        inline bool send(const Json::Value& message) {
            return m_pipe.send_json(message);
        }

    private:
        void bootstrap();
        
        // Thread interoperability
        zmq::context_t& m_context;
        net::json_socket_t m_pipe;
        
        // Data source
        boost::shared_ptr<plugin::source_t> m_source;
        
        // Task persistance
        persistance::storage_t& m_storage;

        // Thread ID
        helpers::auto_uuid_t m_id;
        
        // Thread container
        std::auto_ptr<boost::thread> m_thread;
};

class scheduler_base_t;

// Thread manager
class overseer_t: public boost::noncopyable {
    public:
        overseer_t(zmq::context_t& context, boost::shared_ptr<plugin::source_t> source,
            persistance::storage_t& storage, helpers::auto_uuid_t id);
        
        void run();
        
        // Event loop callbacks
        void request(ev::io& w, int revents);
        void timeout(ev::timer& w, int revents);
        void cleanup(ev::prepare& w, int revents);

        // Event loop binding for schedulers
        inline ev::dynamic_loop& binding() { return m_loop; }
        
        // Caching data fetcher
        plugin::dict_t fetch();
        
        // Scheduler termination request
        void reap(const std::string& scheduler_id);

    private:
        // Command disptach 
        template<class Scheduler>
        void push(const Json::Value& message);
       
        template<class Scheduler>
        void drop(const Json::Value& message);
        
        void once(const Json::Value& message);

        void terminate();

        // Suicide request
        void suicide();

        template<class T>
        inline void respond(const Json::Value& future, const T& value) {
            Json::Value response;
            
            response["future"] = future["id"];
            response["engine"] = m_source->uri();
            response["result"] = value;

            m_futures.send_json(response);
        }

    private:
        // Messaging
        zmq::context_t& m_context;
        net::json_socket_t m_pipe, m_futures, m_reaper;
        
        // Data source
        boost::shared_ptr<plugin::source_t> m_source;
        
        // Task persistance
        persistance::storage_t& m_storage;
      
        // Thread ID
        helpers::auto_uuid_t m_id;

        // Event loop
        ev::dynamic_loop m_loop;
        ev::io m_io;
        ev::timer m_suicide;
        ev::prepare m_cleanup;
        
        // Slaves (Scheduler ID -> Scheduler)
        typedef boost::ptr_map<const std::string, scheduler_base_t> slave_map_t;
        slave_map_t m_slaves;

        // Subscriptions (Scheduler ID -> Tokens)
        typedef std::multimap<const std::string, std::string> subscription_map_t;
        subscription_map_t m_subscriptions;

        // Hasher (for storage)
        helpers::digest_t m_digest;

        // Iteration cache
        bool m_cached;
        plugin::dict_t m_cache;
};

}}}

#endif
