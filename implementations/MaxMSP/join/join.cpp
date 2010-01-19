/* 
 *	join≈
 *	External object for Jamoma Multicore
 *	Copyright © 2008 by Timothy Place
 * 
 *	License: This code is licensed under the terms of the GNU LGPL
 *	http://www.gnu.org/licenses/lgpl.html 
 */

#include "maxMulticore.h"

#define thisTTClass			TTMulticoreJoin
#define thisTTClassName		"multicore.join"
#define thisTTClassTags		"audio, multicore"


/**	The join≈ object takes N input signals and combines them
	into a single signal with all of the channels present.
*/
class TTMulticoreJoin : public TTAudioObject {
	TTCLASS_SETUP(TTMulticoreJoin)
	
public:
	
	TTErr processAudio(TTAudioSignalArrayPtr inputs, TTAudioSignalArrayPtr outputs)
	{
		TTUInt16		numSignals = inputs->numAudioSignals;
		TTUInt16		numAccumulatedChannels = 0;
		TTAudioSignal&	out = outputs->getSignal(0);
		
		for (TTUInt16 i=0; i<numSignals; i++) {
			TTAudioSignal&	in = inputs->getSignal(i);
			
			if (numAccumulatedChannels + in.getNumChannels() > out.getMaxNumChannels())
				out.setAttributeValue(kTTSym_maxNumChannels, numAccumulatedChannels + in.getNumChannels() + 2); 
				// The + 2 above is for padding so that if we come through again then we potentially won't have to alloc memory yet again...

			if (numAccumulatedChannels + in.getNumChannels() > out.getNumChannels())
				out.setAttributeValue(kTTSym_numChannels, numAccumulatedChannels + in.getNumChannels());
			
			TTAudioSignal::copy(in, out, numAccumulatedChannels);
			numAccumulatedChannels += in.getNumChannels();
		}
		return kTTErrNone;
	}
	
};


TT_AUDIO_CONSTRUCTOR_EXPORT
{
	setAttributeValue(TT("maxNumChannels"), arguments);		
	setProcessMethod(processAudio);
}


// Destructor
TTMulticoreJoin::~TTMulticoreJoin()
{
	;
}


/*******************************************************************************/

int main(void)
{
	WrappedClassOptionsPtr	options = new WrappedClassOptions;
	TTValue					value(0);

	TTMulticoreInit();
	TTMulticoreJoin::registerClass();
	
	options->append(TT("argumentDefinesNumInlets"), value);
	return wrapAsMaxMulticore(TT("multicore.join"), "join≈", NULL, options);
}
