/* query.hh
   Queries!

*/

#ifndef MONGOXX_QUERY_HH
#define MONGOXX_QUERY_HH

#include "mapper.hh"
#include "session.hh"

#include <string>
#include <stdexcept>
#include <tr1/memory>

namespace mongoxx {
  class Session;

  class query_error : public std::runtime_error {
  public:
    explicit query_error(std::string const &message) : runtime_error(message) { }
  };

  template <typename T>
  class QueryResult {
  public:
    QueryResult(std::tr1::shared_ptr<mongo::DBClientCursor> const& cursor, Mapper<T> *mapper)
      : m_cursor(cursor), m_mapper(mapper) { }

    T first() const {
      if (m_cursor->more()) {
	mongo::BSONObj obj = m_cursor->next();
	return m_mapper->from_bson(obj);
      }
      throw query_error("Query returned no results; cannot return the first element.");
    }
    T one() const { return first(); }

    bool next(T &t) const {
      if (m_cursor->more()) {
	mongo::BSONObj obj = m_cursor->next();
	m_mapper->from_bson(obj, t);
	return true;
      }
      return false;
    }
    bool first(T &t) const {
      return next(t);
    }
    bool one(T &t) const {
      return next(t);
    }

    bool more() const {
      return m_cursor->more();
    }
    T next() const {
      if (m_cursor->more()) {
	mongo::BSONObj obj = m_cursor->next();
	return m_mapper->from_bson(obj);
      }
      throw query_error("Query results are empty; cannot return any more results.");
    }

    std::vector<T> all() const {
      std::vector<T> res;
      for (T t; next(t); res.push_back(t));
      return res;
    }

  private:
    std::tr1::shared_ptr<mongo::DBClientCursor> m_cursor;
    Mapper<T> *m_mapper;
  };

  template <typename T>
  class Query {
  public:
    Query(Session *session, std::string const& collection, Mapper<T> *mapper)
      : m_session(session), m_collection(collection), m_mapper(mapper) { }

    QueryResult<T> result() const {
      return m_session->execute_query(m_collection, query(), m_mapper);
    }

    T first() const {
      return result().first();
    }

    T one() const {
      return result().one();
    }

    std::vector<T> all() const {
      return result().all();
    }

    void remove_all() const {
      m_session->remove_all(m_collection, query());
    }

    void remove_one() const {
      m_session->remove_one(m_collection, query());
    }

  private:
    Session *m_session;
    std::string m_collection;
    Mapper<T> *m_mapper;

    mongo::Query query() const {
      mongo::BSONObj obj;
      mongo::Query query(obj);
      return query;
    }

  };

};

#endif
