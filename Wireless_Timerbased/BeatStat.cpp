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

#include "BeatStat.h"

using std::ostringstream;
using namespace Marsyas;

BeatStat::BeatStat(mrs_string name) : MarSystem("BeatStat", name)
{
  /// Add any specific controls needed by this MarSystem.
  addControls();
}

BeatStat::BeatStat(const BeatStat& a) : MarSystem(a)
{
  /// All member MarControlPtr have to be explicitly reassigned in
  /// the copy constructor.
  ctrl_mean1_ = getctrl("mrs_natural/mean1");
  ctrl_mean1_ = getctrl("mrs_natural/mean2");
}


BeatStat::~BeatStat()
{
}

MarSystem*
BeatStat::clone() const
{
  return new BeatStat(*this);
}

void
BeatStat::addControls()
{
  /// Add any specific controls needed by this MarSystem.
  addctrl("mrs_natural/mean1", 1000, ctrl_mean1_);
  addctrl("mrs_natural/mean2", 200, ctrl_mean2_);
}

void
BeatStat::myUpdate(MarControlPtr sender)
{
  mean1_ = getctrl("mrs_natural/mean1")->to_natural();
  mean2_ = getctrl("mrs_natural/mean2")->to_natural();

  ctrl_onSamples_->setValue((mrs_natural)1, NOUPDATE);
  ctrl_onObservations_->setValue((mrs_natural)7, NOUPDATE);

  onObservations_ = ctrl_onObservations_->to_natural();
  onSamples_ = ctrl_onSamples_->to_natural();

  ctrl_osrate_->setValue(ctrl_israte_, NOUPDATE);
  ctrl_onObsNames_->setValue("BeatStat_" + ctrl_inObsNames_->to<mrs_string>() , NOUPDATE);
}

void
BeatStat::myProcess(realvec& in, realvec& out)
{
  //mrs_natural t,o;
  counter++;
  /// Iterate over the observations and samples and do the processing.
  if(counter >= mean1_)
  {
	  counter = mean2_;
  }
  mean = (counter-1) * mean/counter + in(0,0)/counter;

  out(0,0) = in(0,0) * 10000;
  out(1,0) = mean * 10000;
  if(out(1,0) < 1.3){
	  //out(1,0) = 1.3;
  }
  out(2,0) = (out(0,0) - out(1,0));
  if(out(2,0) < 0)
      out(2,0) = 0;

  out(3,0) = out(2,0) - old2;
  old2 = old;
  old = out(2,0);




  /*for (o = 0; o < inObservations_; o++)
  {
    for (t = 0; t < inSamples_; t++)
    {
      out(o, t) = in(o, t);
    }
  }*/
}
