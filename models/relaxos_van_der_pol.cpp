/*
 *  *  relaxos_van_der_pol.c
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
#include "relaxos_van_der_pol.h"
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

nest::RecordablesMap<nest::relaxos_van_der_pol> nest::relaxos_van_der_pol::recordablesMap_;

namespace nest
{
  template<>
  void RecordablesMap<relaxos_van_der_pol>::create()
  {
    insert_(names::V_m, &relaxos_van_der_pol::get_x);
    insert_(names::x1, &relaxos_van_der_pol::get_y);
    insert_(names::I, &relaxos_van_der_pol::get_I);
  }

} // namespace


/* ----------------------------------------------------------------
 * Default constructors defining default parameters and state
 * ---------------------------------------------------------------- */

nest::relaxos_van_der_pol::Parameters_::Parameters_()
  : epsilon_ (0),
   I_ex_ (0),
   I_in_ (0),
   x_init_ (0)
{}

nest::relaxos_van_der_pol::State_::State_()
  : x_ (0),
   y_ (0),
   I_ (0)
{}

/* ----------------------------------------------------------------
 * Parameter and state extractions and manipulation functions
 * ---------------------------------------------------------------- */

void nest::relaxos_van_der_pol::Parameters_::Parameters_::get(DictionaryDatum & d) const
{
  def<double>(d, names::epsilon, epsilon_);
  def<double>(d, names::input_currents_ex, I_ex_);
  def<double>(d, names::input_currents_in, I_in_);
  def<double>(d, names::x_init, x_init_);
}

void nest::relaxos_van_der_pol::Parameters_::set(const DictionaryDatum & d)
{
  updateValue<double>(d, names::epsilon, epsilon_);
  updateValue<double>(d, names::input_currents_ex, I_ex_);
  updateValue<double>(d, names::input_currents_in, I_in_);
  updateValue<double>(d, names::x_init, x_init_);
}

void nest::relaxos_van_der_pol::State_::get(DictionaryDatum & d,  const Parameters_ &) const
{
  def<double>(d, names::V_m, x_);
  def<double>(d, names::x1, y_);
  def<double>(d, names::I, I_);
}

void nest::relaxos_van_der_pol::State_::set(const DictionaryDatum & d,  const Parameters_ &)
{
  updateValue<double>(d, names::V_m, x_);
  updateValue<double>(d, names::x1, y_);
  updateValue<double>(d, names::I, I_);
}

nest::relaxos_van_der_pol::Buffers_::Buffers_(relaxos_van_der_pol & n)
  : logger_(n)
{}

nest::relaxos_van_der_pol::Buffers_::Buffers_(const Buffers_ &, relaxos_van_der_pol & n)
  : logger_(n)
{}


/* ----------------------------------------------------------------
 * Default and copy constructor for node
 * ---------------------------------------------------------------- */

nest::relaxos_van_der_pol::relaxos_van_der_pol()
  : Archiving_Node(),
    P_(),
    S_(),
    B_(*this)
{
  recordablesMap_.create();
}

nest::relaxos_van_der_pol::relaxos_van_der_pol(const relaxos_van_der_pol & n)
  : Archiving_Node(n),
    P_(n.P_),
    S_(n.S_),
    B_(n.B_, *this)
{}


/* ----------------------------------------------------------------
 * Node initialization functions
 * ---------------------------------------------------------------- */

void nest::relaxos_van_der_pol::init_state_(const Node & proto)
{
  const relaxos_van_der_pol & pr = downcast<relaxos_van_der_pol>(proto);
  S_ = pr.S_;
}

void nest::relaxos_van_der_pol::init_buffers_()
{
  B_.spikes_.clear();   // includes resize
  B_.currents_.clear(); // includes resize
  B_.logger_.reset();   // includes resize
  Archiving_Node::clear_history();
}

void nest::relaxos_van_der_pol::calibrate()
{
  B_.logger_.init();
  S_.x_ = P_.x_init_;
}


/* ----------------------------------------------------------------
 * Update and spike handling functions
 */

void nest::relaxos_van_der_pol::update(Time const & origin, const long_t from, const long_t to)
{
  assert(to >= 0 && (delay) from < Scheduler::get_min_delay());
  assert(from < to);

  const double_t h = Time::get_resolution().get_ms();

  for (long_t lag = from; lag  < to; lag++)
  {
    double_t x_old, y_old;
    double_t a1 = B_.spikes_.get_value(lag);
    double_t b1 = B_.currents_.get_value(lag);
    x_old = S_.x_ + a1 + S_.I_;
    y_old = S_.y_;

    if (x_old != x_old || y_old != y_old) {
      printf("The system went chaotic and exploded, exiting...\n");
      _exit(0);
    }
    S_.y_ += h*(P_.epsilon_*(1 - x_old*x_old)*y_old - x_old);
    S_.x_ += h*(y_old);

    // send a current event to the network
    CurrentEvent ce;
    ce.set_current(S_.y_);
    network()->send(*this, ce, lag);

    // set new input current
    S_.I_ = b1 + P_.I_ex_ - P_.I_in_;

    // voltage logging
    B_.logger_.record_data(origin.get_steps()+lag);
  }
}

void nest::relaxos_van_der_pol::handle(SpikeEvent & e)
{
  assert(e.get_delay() > 0);
  B_.spikes_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
           e.get_weight() * e.get_multiplicity());
}

void nest::relaxos_van_der_pol::handle(CurrentEvent & e)
{
  assert(e.get_delay() > 0);

  const double_t c=e.get_current();
  const double_t w=e.get_weight();
  B_.currents_.add_value(e.get_rel_delivery_steps(network()->get_slice_origin()),
       w *c);
}

void nest::relaxos_van_der_pol::handle(DataLoggingRequest & e)
{
  B_.logger_.handle(e);
}


