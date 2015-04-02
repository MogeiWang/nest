/*
 *  relaxos_van_der_pol.h
 *
 *  This file is part of NEST.
 *
 *  Copyright (C) 2004 The NEST Initiative
 *
 *  NEST is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  NEST is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with NEST.  If not, see <http://www.gnu.org/licenses/>.
 *
 */


#ifndef RELAXOS_VAN_DER_POL_H
#define RELAXOS_VAN_DER_POL_H


#include "nest.h"
#include "event.h"
#include "archiving_node.h"
#include "ring_buffer.h"
#include "connection.h"
#include "universal_data_logger.h"


namespace nest {
  class Network;

  class relaxos_van_der_pol : public Archiving_Node
  {
    public:
      relaxos_van_der_pol();
      relaxos_van_der_pol(const relaxos_van_der_pol &);

      // bool has_proxies() const {return false;}

      /**
       * Import sets of overloaded virtual functions.
       * @see Technical Issues / Virtual Functions: Overriding, Overloading, and Hiding
       */
      using Node::handle;
      using Node::handles_test_event;

      void handle(SpikeEvent &);
      void handle(CurrentEvent &);
      void handle(DataLoggingRequest &);

      port handles_test_event(SpikeEvent&, rport);
      port handles_test_event(CurrentEvent&, rport);
      port handles_test_event(DataLoggingRequest &, rport);
      port send_test_event(Node&, rport, synindex, bool);

      void get_status(DictionaryDatum &) const;
      void set_status(const DictionaryDatum &);

    private:
      void init_state_(const Node& proto);
      void init_buffers_();
      void calibrate();

      void update(Time const &, const long_t, const long_t);

      // The next two classes need to be friends to access the State_ class/member
      friend class RecordablesMap<relaxos_van_der_pol>;
      friend class UniversalDataLogger<relaxos_van_der_pol>;

      struct Parameters_ {
        double_t epsilon_;
        double_t I_ex_;
        double_t I_in_;
        double_t x_init_;

        Parameters_();

        void get(DictionaryDatum&) const;
        void set(const DictionaryDatum&);
      };

      struct State_ {
        double_t x_;
        double_t y_;
        double_t I_;

        State_();  //!< Default initialization

        void get(DictionaryDatum&, const Parameters_&) const;
        void set(const DictionaryDatum&, const Parameters_&);
      };

      struct Buffers_ {
        Buffers_(relaxos_van_der_pol&);
        Buffers_(const Buffers_&, relaxos_van_der_pol&);

        /** buffers and summs up incoming spikes/currents */
        RingBuffer spikes_;
        RingBuffer currents_;

        //! Logger for all analog data
        UniversalDataLogger<relaxos_van_der_pol> logger_;
      };

      struct Variables_ {
      };

      Parameters_ P_;
      State_      S_;
      Variables_  V_;
      Buffers_    B_;

      double_t get_x() const { return S_.x_; }
      double_t get_y() const { return S_.y_; }
      double_t get_I() const { return S_.I_; }

      //! Mapping of recordables names to access functions
      static RecordablesMap<relaxos_van_der_pol> recordablesMap_;
  };

  inline
  port relaxos_van_der_pol::send_test_event(Node& target, rport receptor_type, synindex, bool)
  {
    SpikeEvent e;
    e.set_sender(*this);

    return target.handles_test_event(e, receptor_type);
  }

  inline
  port relaxos_van_der_pol::handles_test_event(SpikeEvent&, rport receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return 0;
  }

  inline
  port relaxos_van_der_pol::handles_test_event(CurrentEvent&, rport receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return 0;
  }

  inline
  port relaxos_van_der_pol::handles_test_event(DataLoggingRequest &dlr, rport receptor_type)
  {
    if (receptor_type != 0)
      throw UnknownReceptorType(receptor_type, get_name());
    return B_.logger_.connect_logging_device(dlr, recordablesMap_);
  }

  inline
  void relaxos_van_der_pol::get_status(DictionaryDatum &d) const
  {
    P_.get(d);
    S_.get(d, P_);
    Archiving_Node::get_status(d);
    (*d)[names::recordables] = recordablesMap_.get_list();
  }

  inline
  void relaxos_van_der_pol::set_status(const DictionaryDatum &d)
  {
    Parameters_ ptmp = P_;  // temporary copy in case of errors
    ptmp.set(d);                       // throws if BadProperty
    State_      stmp = S_;  // temporary copy in case of errors
    stmp.set(d, ptmp);                 // throws if BadProperty

    // We now know that (ptmp, stmp) are consistent. We do not
    // write them back to (P_, S_) before we are also sure that
    // the properties to be set in the parent class are internally
    // consistent.
    Archiving_Node::set_status(d);

    // if we get here, temporaries contain consistent set of properties
    P_ = ptmp;
    S_ = stmp;
  }

} // namespace nest


#endif


