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

#include "BeatRec.h"

#include "marsyas/common_source.h"

using std::ostringstream;
using namespace Marsyas;

BeatRec::BeatRec(mrs_string name) : MarSystem("BeatRec", name)
{
  /// Add any specific controls needed by this MarSystem.
  bufferSize = 5;
  buffer.create(1,5);
  oldTime = std::chrono::system_clock::now();
  newTime = oldTime;
  addControls();
  generator = 0;

}

BeatRec::BeatRec(const BeatRec& a) : MarSystem(a)
{
  /// All member MarControlPtr have to be explicitly reassigned in
  /// the copy constructor.
  ctrl_gain_EXAMPLE_ = getctrl("mrs_real/gain");
  ctrl_bufferSize_ = getctrl("mrs_natural/bufferSize");
  ctrl_threshold_ = getctrl("mrs_real/threshold");

}


BeatRec::~BeatRec()
{
}

MarSystem*
BeatRec::clone() const
{
  //controller = new WSControll(20, 10, 4);
  return new BeatRec(*this);
}
void BeatRec::initSignal(BehaviourGenerator *igenerator){
	this->generator = igenerator;
	this->sig.connect(BehaviourGenerator::beat);
}
void
BeatRec::addControls()
{
  /// Add any specific controls needed by this MarSystem.

  addctrl("mrs_bool/dummyEXAMPLE", false);
  setctrlState("mrs_bool/dummyEXAMPLE", true);
  addctrl("mrs_natural/bufferSize", 5, ctrl_bufferSize_);
  addctrl("mrs_real/gain", 1.0, ctrl_gain_EXAMPLE_);
  addctrl("mrs_real/threshold", 500.0 ,ctrl_threshold_);
}

void
BeatRec::myUpdate(MarControlPtr sender)
{
  (void) sender;
  ctrl_onSamples_->setValue(ctrl_inSamples_, NOUPDATE);
  ctrl_onObservations_->setValue(ctrl_inObservations_, NOUPDATE);
  ctrl_osrate_->setValue(ctrl_israte_, NOUPDATE);

  threshold = getctrl("mrs_real/threshold")->to_real();
  bufferSize = getctrl("mrs_natural/bufferSize")->to_natural();

  buffer.create(4,1);
  buffer(1,0) = 500;
  ufaktor = 64.0/48000.0 * 1000.0;
  frontier = (int)(buffer(1,0)/ufaktor);
  min = 0;
  max = 0;
  autoset = 0.8;
  autoset2 = 0.8;
  autoset3 = 0.8;
  //setctrl("mrs_natural/onSamples", upperbin - lowerbin);
  /// Use the default MarSystem setup with equal input/output stream format.
  //MarSystem::myUpdate(sender);
}

void
BeatRec::myProcess(realvec& in, realvec& out)
{
  //mrs_real mean = 0;
  //mrs_natural t,o;
  int teiler;
  //int multiplier;
  double rest;
  counter++;

  if(counter == frontier - 10){ //20
	  autoset = 0.5;
	  autoset2 = 0.5;
	  autoset3 = 0.5;
  }
  else if(counter == 1){
	  autoset = 18;
	  autoset2 = 18;
	  autoset3 = 18;
  }
  else if(counter == frontier + 10)
  {
	  autoset = 1.2;
	  autoset2 = 1.2;
	  autoset3 = 1.2;
  }
  else if(counter == 2 * frontier - 5)
  {
  	  autoset = 0.6;
  	  autoset2 = 0.6;
  	  autoset3 = 0.6;
  }
  else if(counter == 2*frontier + 20)
  {
  	  autoset = 0.9;
  	  autoset2 = 0.9;
  	  autoset3 = 0.6;
  }
  if(buffer(0,0) == 0)
      buffer(0,0) = in(0,0);
  if(buffer(2,0) == 0)
      		  buffer(2,0) = in(2,0);
  if(buffer(3,0) == 0)
      		  buffer(3,0) = in(3,0);
  if(std::abs(in(0,0)/buffer(0,0))> autoset && std::abs(in(2,0)/buffer(2,0)) > autoset2 && std::abs(in(3,0)/buffer(3,0)) > autoset3)
  {
	  newTime = std::chrono::system_clock::now();
	  ms = std::chrono::duration_cast<std::chrono::milliseconds>(newTime - oldTime);
	  teiler = std::round((double)ms.count()/buffer(1,0));
	  rest = ms.count() - buffer(1,0) * teiler;
	  if(counter > 300){
		  oldTime = newTime;
	  }


      out(0,0) = 1;
      sig(generator, buffer(1,0), std::abs(in(0,0)/buffer(0,0)));
      MRSERR("Beatpower (%): " << (std::abs(in(0,0)/buffer(0,0)) * 100) << "\tBeatdistindtiveness (%): " << (std::abs(in(2,0)/buffer(2,0)) * 100) << "\tBeatslope (%): "<< (std::abs(in(3,0)/buffer(3,0))*100) << "\tEstimate BPM: "<< std::abs(1/buffer(1,0)*60000) << "\tBeatTimeDistance (ms): " << ms.count());
      if(ms.count() > 400){

    	  buffer(0,0) = 40*buffer(0,0)/50 + 10*in(0,0)/50;


    	  if(bpmcounter == 40){

    		  bpmcounter = 21;
    	  }
    	  else if(bpmcounter < 12)
    	  {
    		  buffer(1,0) = 500;
    		  bpmcounter++;
    	  }
    	  else
    	  {
    		  buffer(1,0) = (bpmcounter-1)*buffer(1,0)/bpmcounter + (buffer(1,0)+rest)/bpmcounter;
    		  if(1/buffer(1,0)*60000 > 135)
    			  buffer(1,0) = 60000/125;
    		  if(1/buffer(1,0)*60000 < 118)
    			  buffer(1,0) = 60000/120;
    	      bpmcounter++;
    	  }

    	  buffer(2,0) = 40*buffer(2,0)/50 + 10*in(2,0)/50;
    	  buffer(3,0) = 40*buffer(3,0)/50 + 10*in(3,0)/50;

      }

      frontier = buffer(1,0) / ufaktor;
      counter = 0;



  }

  else
      out(0,0) = 0;
}
