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

#include "BeatByNiko.h"

#include <marsyas/common_source.h>
#include <math.h>

using std::ostringstream;
using namespace Marsyas;

BeatByNiko::BeatByNiko(mrs_string name) : MarSystem("BeatByNiko", name)
{
  /// Add any specific controls needed by this MarSystem.
  addControls();
}

BeatByNiko::BeatByNiko(const BeatByNiko& a) : MarSystem(a)
{
  /// All member MarControlPtr have to be explicitly reassigned in
  /// the copy constructor.
}


BeatByNiko::~BeatByNiko()
{
}

MarSystem*
BeatByNiko::clone() const
{
  return new BeatByNiko(*this);
}

void
BeatByNiko::addControls()
{
  /// Add any specific controls needed by this MarSystem.


  addctrl("mrs_natural/meanSamples", 5, ctrl_meanSamples);
  addctrl("mrs_real/threshold", 1.0, ctrl_threshold);
  addctrl("mrs_real/lowercutoff", 50.0, ctrl_lowercutofffreq);
  addctrl("mrs_real/uppercutoff", 1000.0, ctrl_uppercutofffreq);
  addctrl("mrs_real/frequency", 48000.0, ctrl_freq);

}

void
BeatByNiko::myUpdate(MarControlPtr sender)
{
  (void) sender;
  MRSDIAG("BeatByNiko.cpp - BeatByNiko:myUpdate");
  ctrl_onSamples_->setValue((mrs_natural)1, NOUPDATE);
  ctrl_onObservations_->setValue((mrs_natural)7, NOUPDATE);

  onObservations_ = ctrl_onObservations_->to_natural();



  meanlt = 0;
  counter = 0;


  meanSamples = getctrl("mrs_natural/meanSamples")->to_natural();
  threshold = getctrl("mrs_real/threshold")->to_real();

  lowercutoff = getctrl("mrs_real/lowercutoff")->to_real();
  uppercutoff = getctrl("mrs_real/uppercutoff")->to_real();

  ///Calculate cutoffbin from freq

  lowercbin = lowercutoff * ctrl_inObservations_->to_natural() / ctrl_israte_->to_real();
  uppercbin = uppercutoff * ctrl_inObservations_->to_natural() / ctrl_israte_->to_real();

  temp.create(400);

  buffer.create(meanSamples);
  ctrl_osrate_->setValue(ctrl_israte_, NOUPDATE);
  ctrl_onObsNames_->setValue("BeatNiko_" + ctrl_inObsNames_->to<mrs_string>() , NOUPDATE);
}

void
BeatByNiko::myProcess(realvec& in, realvec& out)
{
  if(counter == 1000)
      counter = 700;
  mrs_natural o;
  mrs_real outr;
  mrs_real tmp = 0;
  mrs_real outr2;
  ///integrieren und letztes sample abspeichern
  mrs_real length = (mrs_real)uppercbin-lowercbin;
  for (o = lowercbin; o < uppercbin; o++)
  {
    outr2 += (in(o,0)+90)*(in(o,0)+90)/length;
  }
  outr = std::sqrt(outr2);

  counter += 1;
  if(outr > 0)
    meanlt = (counter-1)*(meanlt/counter) + outr2/counter;
  else
      meanlt = (counter-1)*(meanlt/counter) + 0/counter;

  ///Mittelwert berechnen
  for(int i = 0; i < meanSamples -1; i++){
      buffer(i+1) = buffer(i);
  }
  ///Checke auf Onset sonst null ausgeben
  buffer(0) = outr2;

  ///Ausgabe
  tmp = outr2;

  out(0,0) = tmp;

  out(1,0) = outr2 - meanlt;
  if(out(1,0) < 0)
      out(1,0) = 0;


  out(2,0) = out(1,0) - old;
  old = out(1,0);
  out(4,0) = meanlt;
}
