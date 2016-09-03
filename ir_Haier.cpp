/*
Assuming the protocol we are adding is for the (imaginary) manufacturer:  Haier

Our fantasy protocol is a standard protocol, so we can use this standard
template without too much work. Some protocols are quite unique and will require
considerably more work in this file! It is way beyond the scope of this text to
explain how to reverse engineer "unusual" IR protocols. But, unless you own an
oscilloscope, the starting point is probably to use the rawDump.ino sketch and
try to spot the pattern!

Before you start, make sure the IR library is working OK:
  # Open up the Arduino IDE
  # Load up the rawDump.ino example sketch
  # Run it

Now we can start to add our new protocol...

1. Copy this file to : ir_HAIER.cpp

2. Replace all occurrences of "Haier" with the name of your protocol.

3. Tweak the #defines to suit your protocol.

4. If you're lucky, tweaking the #defines will make the default send() function
   work.

5. Again, if you're lucky, tweaking the #defines will have made the default
   decode() function work.

You have written the code to support your new protocol!

Now you must do a few things to add it to the IRremote system:

1. Open IRremote.h and make the following changes:
   REMEMEBER to change occurences of "Haier" with the name of your protocol

   A. At the top, in the section "Supported Protocols", add:
      #define DECODE_HAIER  1
      #define SEND_HAIER    1

   B. In the section "enumerated list of all supported formats", add:
      Haier,
      to the end of the list (notice there is a comma after the protocol name)

   C. Further down in "Main class for receiving IR", add:
      //......................................................................
      #if DECODE_HAIER
          bool  decodeHaier (decode_results *results) ;
      #endif

   D. Further down in "Main class for sending IR", add:
      //......................................................................
      #if SEND_HAIER
          void  sendHaier (unsigned long data,  int nbits) ;
      #endif

   E. Save your changes and close the file

2. Now open irRecv.cpp and make the following change:

   A. In the function IRrecv::decode(), add:
      #ifdef DECODE_NEC
          DBG_PRINTLN("Attempting Haier decode");
          if (decodeHaier(results))  return true ;
      #endif

   B. Save your changes and close the file

You will probably want to add your new protocol to the example sketch

3. Open MyDocuments\Arduino\libraries\IRremote\examples\IRrecvDumpV2.ino

   A. In the encoding() function, add:
      case Haier:    Serial.print("Haier");     break ;

Now open the Arduino IDE, load up the rawDump.ino sketch, and run it.
Hopefully it will compile and upload.
If it doesn't, you've done something wrong. Check your work.
If you can't get it to work - seek help from somewhere.

If you get this far, I will assume you have successfully added your new protocol
There is one last thing to do.

1. Delete this giant instructional comment.

2. Send a copy of your work to us so we can include it in the library and
   others may benefit from your hard work and maybe even write a song about how
   great you are for helping them! :)

Regards,
  BlueChip
*/

#include "IRremote.h"
#include "IRremoteInt.h"

//==============================================================================
//
//
//                              S H U Z U
//
//
//==============================================================================

#define BITS          112  // 27 * 4 + 4 = 112 fail

#define HDR_MARK    3050  // The length of the Header:Mark
#define HDR_SPACE   3050  // The lenght of the Header:Space
#define HDR_MARK_2    3050  // The length of the Header:Mark
#define HDR_SPACE_2   4350  // The lenght of the Header:Space

#define BIT_MARK    600  // The length of a Bit:Mark
#define ONE_SPACE   1690  // The length of a Bit:Space for 1's
#define ZERO_SPACE  600  // The length of a Bit:Space for 0's

//+=============================================================================
//
#if SEND_HAIER
void  IRsend::sendHaier (unsigned long data,  int nbits)
{
	// Set IR carrier frequency
	enableIROut(38);

	// Header
	mark (HDR_MARK);
	space(HDR_SPACE);
    mark (HDR_MARK_2);
    space(HDR_SPACE_2);

	// Data
	for (unsigned long  mask = 1UL << (nbits - 1);  mask;  mask >>= 1) {
		if (data & mask) {
			mark (BIT_MARK);
			space(ONE_SPACE);
		} else {
			mark (BIT_MARK);
			space(ZERO_SPACE);
		}
	}

	// Footer
	mark(BIT_MARK);
    space(0);  // Always end with the LED off
}
#endif

//+=============================================================================
//
#if DECODE_HAIER
bool  IRrecv::decodeHaier (decode_results *results)
{
    Serial.println("start decode.");
	unsigned long  data   = 0;  // Somewhere to build our code
	int            offset = 1;  // Skip the Gap reading
    

	// Check we have the right amount of data
    Serial.println(irparams.rawlen);
	if (irparams.rawlen != 2 + 2 + (2 * BITS) + 2)  return false ;
Serial.println("irparams.rawlen ok.");
	// Check initial Mark+Space match
	if (!MATCH_MARK (results->rawbuf[offset++], HDR_MARK ))  return false ;
	if (!MATCH_SPACE(results->rawbuf[offset++], HDR_SPACE))  return false ;
    if (!MATCH_MARK(results->rawbuf[offset++], HDR_MARK_2 ))  return false ;
    if (!MATCH_SPACE(results->rawbuf[offset++], HDR_SPACE_2))  return false ;
    Serial.println("MATCH_MARK ok.");
    
	// Read the bits in
	for (int i = 0;  i < BITS;  i++) {
		// Each bit looks like: MARK + SPACE_1 -> 1
		//                 or : MARK + SPACE_0 -> 0
        
		 if (!MATCH_SPACE(results->rawbuf[offset++], BIT_MARK))  return false ;
        //MATCH_MARK() is useless.
        
		// IR data is big-endian, so we shuffle it in from the right:
		if      (MATCH_SPACE(results->rawbuf[offset], ONE_SPACE))   data = (data << 1) | 1 ;
		else if (MATCH_SPACE(results->rawbuf[offset], ZERO_SPACE))  data = (data << 1) | 0 ;
		else                                                        return false ;
            Serial.print("repeat check ok- ");
            Serial.println(data, HEX);
        
		offset++;
	}
    
    Serial.println("Read the bits ok.");
	// Success
	results->bits        = BITS;
	results->value       = data;
	results->decode_type = HAIER;
	return true;
}
#endif
