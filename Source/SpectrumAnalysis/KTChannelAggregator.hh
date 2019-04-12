/*
 * KTChannelAggregator.hh
 *
 *  Created on: Jan 25, 2019
 *      Author: P. T. Surukuchi
 */

#ifndef KTCHANNELAGGREGATOR_HH_
#define KTCHANNELAGGREGATOR_HH_

#include "KTProcessor.hh"
#include "KTData.hh"
#include "KTPowerSpectrumData.hh"
#include "KTChannelAggregatedData.hh"
#include "KTFrequencySpectrumDataFFTW.hh"
#include "KTAxisProperties.hh"

#include "KTSlot.hh"

#include "KTMath.hh"

namespace Katydid
{
  
  KTLOGGER(avlog_hh, "KTChannelAggregator.hh");
  
  class KTChannelAggregatedData;
  class KTPowerSpectrumData;
  
  /*
   @class KTChannelAggregator
   @author P. T. Surukuchi
   
   @brief Multiple channel summation for Phase-III and IV
   
   @details
   More details to come.
   
   Configuration name: "channel-aggregator"
   
   Slots:
   - "fft": void (Nymph::KTDataPtr) -- Adds channels voltages using FFTW-phase information for appropriate phase addition; Requires KTFrequencySpectrumDataFFTW; Adds summation of the channel results; Emits signal "fft"
   
   Signals:
   - "fft": void (Nymph::KTDataPtr) -- Emitted upon summation of all channels; Guarantees KTFrequencySpectrumDataFFTW
   */
  
  class KTChannelAggregator : public Nymph::KTProcessor
  {
  public:
    KTChannelAggregator(const std::string& name = "channel-aggregator");
    virtual ~KTChannelAggregator();
    
    bool Configure(const scarab::param_node* node);
    
    /*
     Set radius of the active area assuming a circular active area.
     */
    virtual void SetActiveRadius(double);
    
    /*
     Get radius of the active area.
     */
    virtual double GetActiveRadius() const;
    
    bool GetIsGridDefined() const;
  private:
    
    // in meters, should not be hard-coded
    // PTS: Has to come as an input from config file ?
    double fActiveRadius = 0.0516;
    
    //For exception handling to make sure the grid is defined before the spectra are assigned.
    bool fIsGridDefined=false;
    
    bool SumChannelVoltageWithPhase( KTFrequencySpectrumDataFFTW& );
    
    /// Returns the phase shift based on a given point, angle of the channel and the wavelength
    double GetPhaseShift(double,double,double,double);
    
    /// Get location of the point in the grid based on the given grid number and the size of the grid.
    /* Returns true if the assigment went well, false if there was some mistake
     */
    bool GetGridLocation(int,int,double &);
    
    /// Convert frquency to wavlength
    double ConvertFrequencyToWavelength(double);
    
  private:
    //PTS: This needs fixing, currently just setting each element to 0. But why does it have to be done to begin with.
    // Perhaps there is some function in the utilities to do this ?
    bool NullFreqSpectrum(KTFrequencySpectrumFFTW &);
    
    //***************
    // Signals
    //***************
    
  private:
    Nymph::KTSignalData fSummedFrequencyData;
    
    //***************
    // Slots
    //***************
    
  private:
    Nymph::KTSlotDataOneType< KTFrequencySpectrumDataFFTW > fPhaseChFrequencySumSlot;
  };
  
  inline bool KTChannelAggregator::NullFreqSpectrum(KTFrequencySpectrumFFTW &freqSpectrum)
  {
    for (unsigned i = 0; i < freqSpectrum.size(); ++i)
    {
      freqSpectrum.SetRect(i,0.0,0.0);
    }
    return true;
  }
  
  inline double KTChannelAggregator::GetActiveRadius() const
  {
    return fActiveRadius;
  }
  
  inline void KTChannelAggregator::SetActiveRadius(double radius)
  {
    fActiveRadius=radius;
    return;
  }
  
  inline bool KTChannelAggregator::GetIsGridDefined() const
  {
    return fIsGridDefined;
  }
}

#endif  /* KTCHANNELAGGREGATOR_HH_  */
