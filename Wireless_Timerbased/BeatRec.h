/*
** Copyright (C) 1998-2010 George Tzanetakis <gtzan@cs.uvic.ca>
**
** This program is free software; you can redistribute it and/or modify
** it under the terms of the GNU General Public License as published by
** the Free Software Foundation; either version 2 of the License, or
** (at your option) any later version.
**
** This program is distributed in the hope that it will be useful,
** but WITHOUT ANY WARRANTY; without even the implied warranty of
** MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
** GNU General Public License for more details.
**
** You should have received a copy of the GNU General Public License
** along with this program; if not, write to the Free Software
** Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

#ifndef MARSYAS_BEATREC_H
#define MARSYAS_BEATREC_H

#include <marsyas/system/MarSystem.h>
#include <boost/thread/mutex.hpp>
#include "BehaviourGenerator.h"
#include <boost/signals2.hpp>
#include <boost/chrono.hpp>
#include <chrono>
#include <iostream>
namespace Marsyas
{
/**
	\class BeatRec
	\ingroup Processing
	\brief Basic example on how to use controls efficiently in MarSystems

	This example is the same as Gain; it scales the output by
	multiplying each sample with a real number.

	Controls:
	- \b mrs_real/gain [w] : sets the gain multiplier.
	- \b mrs_bool/dummy [rw] : does nothing.
*/

class marsyas_EXPORT BeatRec: public MarSystem
{
private:

  /// Add specific controls needed by this MarSystem.
  void addControls();

  /// Reads changed controls and sets up variables if necessary.
  void myUpdate(MarControlPtr sender);


  /// MarControlPtr for the gain control

  MarControlPtr ctrl_gain_EXAMPLE_;
  MarControlPtr ctrl_threshold_;
  MarControlPtr ctrl_bufferSize_;
  mrs_real threshold;
  mrs_natural beat;
  mrs_natural bufferSize;
  mrs_realvec buffer;
  mrs_real min;
  mrs_real max;
  mrs_natural counter;
  mrs_real autoset;
  mrs_real autoset2;
  mrs_real autoset3;
  std::chrono::system_clock::time_point newTime;
  std::chrono::system_clock::time_point oldTime;
  std::chrono::milliseconds ms;
  mrs_natural bpmcounter;
  mrs_natural frontier;
  mrs_real ufaktor;

public:
  BehaviourGenerator *generator;
  boost::signals2::signal<void (BehaviourGenerator *generator, double ibpmTime, double ipower)> sig;
  void initSignal(BehaviourGenerator *igenerator);
  /// BeatRec constructor.
  BeatRec(std::string name);

  /// BeatRec copy constructor.
  BeatRec(const BeatRec& a);

  /// BeatRec destructor.
  ~BeatRec();

  /// Implementation of the MarSystem::clone() method.
  MarSystem* clone() const;

  /// Implementation of the MarSystem::myProcess method.
  void myProcess(realvec& in, realvec& out);
};

}
//namespace Marsyas

#endif
//MARSYAS_BEATREC_H

