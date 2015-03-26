/*
 *  simple.cpp
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

#include "exceptions.h"
#include "simple.h"
#include "network.h"
#include "dict.h"
#include "integerdatum.h"
#include "doubledatum.h"
#include "dictutils.h"
#include "numerics.h"
#include "universal_data_logger_impl.h"


/* ----------------------------------------------------------------
 * Recordables map
 * ---------------------------------------------------------------- */

nest::RecordablesMap<nest::simple> nest::simple::recordablesMap_;

namespace nest
{
  template<>
  void RecordablesMap<simple>::create()
  {
    insert_(names::spikes, &simple::get_spikes);
  }

} // namespace


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::simple::Parameters_::Parameters_()
  : thresh (2)
{}

nest::simple::State_::State_()
  : spikes (0)
{}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void nest::simple::Parameters_::Parameters_::get(DictionaryDatum & d) const
{
  def<double>(d, names::thresh, thresh);
}

void nest::simple::Parameters_::set(const DictionaryDatum & d)
{
  updateValue<double>(d, names::thresh, thresh);
}

void nest::simple::State_::get(DictionaryDatum & d,  const Parameters_ &) const
{
  def<double>(d, names::spikes, spikes);
}

void nest::simple::State_::set(const DictionaryDatum & d,  const Parameters_ &)
{
  updateValue<double>(d, names::spikes, spikes);
}

nest::simple::Buffers_::Buffers_(simple & n)
  : logger_(n)
{}

nest::simple::Buffers_::Buffers_(const Buffers_ &, simple & n)
  : logger_(n)
{}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::simple::simple()
  : Archiving_Node(),
    P_(),
    S_(),
    B_(*this)
{
  recordablesMap_.create();
}

nest::simple::simple(const simple & n)
  : Archiving_Node(n),
    P_(n.P_),
    S_(n.S_),
    B_(n.B_, *this)
{}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void nest::simple::init_state_(const Node & proto)
{
  const simple & pr = downcast<simple>(proto);
  S_ = pr.S_;
}

void nest::simple::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
  Archiving_Node::clear_history();
}

void nest::simple::calibrate()
{
  B_.logger_.init();
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void nest::simple::update(Time const & origin, const long_t from, const long_t to)
{
  assert(to >= 0 && (delay) from < Scheduler::get_min_delay());
  assert(from < to);

  for (long_t lag = from; lag  < to; lag++)
  {
    double_t s = B_.spikes_.get_value(lag);
    if (s > P_.thresh)
    {
      // threshold reached, spike
      set_spiketime(Time::step(origin.get_steps()+lag+1));

      SpikeEvent se;
      network()->send(*this, se, lag);
    }

  B_.logger_.record_data(origin.get_steps()+lag);

  }
}


void nest::simple::handle(SpikeEvent & e)
{
  assert(e.get_delay() > 0);
  B_.spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
           e.get_weight() * e.get_multiplicity());
}

void nest::simple::handle(CurrentEvent & e)
{
  assert(e.get_delay() > 0);

  const double_t c=e.get_current();
  const double_t w=e.get_weight();
  B_.currents_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
       w *c);
}

void nest::simple::handle(DataLoggingRequest & e)
{
  B_.logger_.handle(e);
}

