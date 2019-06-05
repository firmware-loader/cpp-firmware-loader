/*******************************************************************************
*
* INTEL HEX FILE CLASS MODULE
*
*******************************************************************************/

/******************************************************************************/
/*!
* \file intelhexclass.cpp
* \mainpage
* \image html intelhexclass.png
* \image latex intelhexclass.eps
* \section intro Introduction
* The Intel HEX File class module is designed to encode, decode and manipulate
* the content of Intel HEX format files commonly generated by most toolchains
* for embedded processors and microcontrollers.
*
* It uses standard C++ streams to decode files and store them in memory, and
* encode data stored in memory back into an Intel HEX format file. Once the file
* content is in memory, the content can then be manipulated using the available
* API.
*
* With this class it is possible to create tools that can compare Intel HEX
* files, fill empty space with desired values, splice two or more files
* together to name a few possibilities.
*
* \section contactInfo Contact Information
* For more information and the latest release, please visit this projects home
* page at http://codinghead.github.com/Intel-HEX-Class
* To participate in the project or for other enquiries, please contact Stuart
* Cording at codinghead@gmail.com
*
* \section license Licensing Information
* Copyright (c) 2012 Stuart Cording
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in all
* copies or substantial portions of the Software.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
* AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
* LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
* OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* \section imageInfo Image Information
* Image chosen for this project comes from 'Henkster'. Original image is from
* http://www.sxc.hu/photo/504350 on stock.xchng.
*
* \author Stuart Cording aka CODINGHEAD
*
********************************************************************************
* \note
* No notes to date (19th Jan 2012)
*******************************************************************************/

#include <iostream>
#include <string>
#include <vector>
#ifdef _MSC_FULL_VER
#include <stdio.h>
#else
#include <cstdio>
#endif

#include "intelhexclass.h"

//using namespace std;

/******************************************************************************/
/*! Possible record types for Intel HEX file.
*
* List of all possible record types that can be found in an Intel HEX file.
*******************************************************************************/
enum intelhexRecordType {
	DATA_RECORD,                 // '00'
	END_OF_FILE_RECORD,          // '01'
	EXTENDED_SEGMENT_ADDRESS,    // '02'
	START_SEGMENT_ADDRESS,       // '03'
	EXTENDED_LINEAR_ADDRESS,     // '04'
	START_LINEAR_ADDRESS,        // '05'
	NO_OF_RECORD_TYPES
};

/*******************************************************************************
* Converts a 2 char string to its HEX value
*******************************************************************************/
unsigned char intelhex::stringToHex(std::string value)
{
	unsigned char returnValue = 0;
	std::string::iterator valueIterator;

	if (value.length() == 2)
	{
		valueIterator = value.begin();

		for (int x = 0; x < 2; x++)
		{
			/* Shift result variable 4 bits to the left                       */
			returnValue <<= 4;

			if (*valueIterator >= '0' && *valueIterator <= '9')
			{
				returnValue +=
					static_cast<unsigned char>(*valueIterator - '0');
			}
			else if (*valueIterator >= 'A' && *valueIterator <= 'F')
			{
				returnValue +=
					static_cast<unsigned char>(*valueIterator - 'A' + 10);
			}
			else if (*valueIterator >= 'a' && *valueIterator <= 'f')
			{
				returnValue +=
					static_cast<unsigned char>(*valueIterator - 'a' + 10);
			}
			else
			{
				/* Error occured - non-HEX value found                        */
				std::string message;

				message = "Can't convert byte 0x" + value + " @ 0x" +
					ulToHexString(segmentBaseAddress) + " to hex.";

				addError(message);

				returnValue = 0;
			}

			/* Iterate to next char in the string                             */
			++valueIterator;
		}
	}
	else
	{
		/* Error occured - more or less than two nibbles in the string        */
		std::string message;

		message = value + " @ 0x" + ulToHexString(segmentBaseAddress) +
			" isn't an 8-bit value.";

		addError(message);
	}

	return returnValue;
}

/*******************************************************************************
* Converts an unsigned long to a string in HEX format
*******************************************************************************/
std::string intelhex::ulToHexString(unsigned long value)
{
	std::string returnString;
	char localString[50];

	returnString.erase();

#ifdef _MSC_FULL_VER
	sprintf_s(localString, 49, "%08lX", value);
#else
	snprintf(localString, 49, "%08lX", value);
#endif

	returnString.insert(0, localString);

	return returnString;
}

/*******************************************************************************
* Converts an unsigned long to a string in DEC format
*******************************************************************************/
std::string intelhex::ulToString(unsigned long value)
{
	std::string returnString;
	char localString[50];

	returnString.erase();

#ifdef _MSC_FULL_VER
	sprintf_s(localString, 49, "%lu", value);
#else
	snprintf(localString, 49, "%lu", value);
#endif
	returnString.insert(0, localString);

	return returnString;
}

/*******************************************************************************
* Converts an unsigned char to a string in HEX format
*******************************************************************************/
std::string intelhex::ucToHexString(unsigned char value)
{
	std::string returnString;
	char localString[50];

	returnString.erase();

#ifdef _MSC_FULL_VER
	sprintf_s(localString, 49, "%02X", value);
#else
	snprintf(localString, 49, "%02X", value);
#endif

	returnString.insert(0, localString);

	return returnString;
}

/*******************************************************************************
* Adds a warning to the list of warning messages
*******************************************************************************/
void intelhex::addWarning(std::string warningMessage)
{
	std::string localMessage;

	/* Build the message and push the warning message onto the list           */
	localMessage += ulToString(msgWarning.noOfWarnings + 1) + " Warning: "
		+ warningMessage;

	msgWarning.ihWarnings.push_back(localMessage);

	/* Update the number of warning messages                                  */
	msgWarning.noOfWarnings = msgWarning.ihWarnings.size();
}

/*******************************************************************************
* Adds an error to the list of error messages
*******************************************************************************/
void intelhex::addError(std::string errorMessage)
{
	std::string localMessage;

	/* Build the message and push the error message onto the list             */
	localMessage += ulToString(msgError.noOfErrors + 1) + " Error: "
		+ errorMessage;

	msgError.ihErrors.push_back(localMessage);

	/* Update the number of error messages                                    */
	msgError.noOfErrors = msgError.ihErrors.size();
}

/*******************************************************************************
* Decodes a data record read in from a file
*******************************************************************************/
void intelhex::decodeDataRecord(unsigned char recordLength,
	unsigned long loadOffset,
	std::string::const_iterator data)
{
	/* Variable to store a byte of the record as a two char string            */
	std::string sByteRead;

	/* Variable to store the byte of the record as an u.char                  */
	unsigned char byteRead;

	/* Calculate new SBA by clearing the low four bytes and then adding the   */
	/* current loadOffset for this line of Intel HEX data                     */
	segmentBaseAddress &= ~(0xFFFFUL);
	segmentBaseAddress += loadOffset;

	for (unsigned char x = 0; x < recordLength; x++)
	{
		sByteRead.erase();

		sByteRead = *data;
		data++;
		sByteRead += *data;
		data++;

		byteRead = stringToHex(sByteRead);

		ihReturn = ihContent.insert(
			std::pair<unsigned long, unsigned char>(segmentBaseAddress, byteRead));

		if (ihReturn.second == false)
		{
			/* If this address already contains the byte we are trying to     */
			/* write, this is only a warning                                  */
			if (ihReturn.first->second == byteRead)
			{
				std::string message;

				message = "Location 0x" + ulToHexString(segmentBaseAddress) +
					" already contains data 0x" + sByteRead;

				addWarning(message);
			}
			/* Otherwise this is an error                                     */
			else
			{
				std::string message;

				message = "Couldn't add 0x" + sByteRead + " @ 0x" +
					ulToHexString(segmentBaseAddress) +
					"; already contains 0x" +
					ucToHexString(ihReturn.first->second);

				addError(message);
			}
		}

		/* Increment the segment base address                                 */
		++segmentBaseAddress;
	}
}

/*******************************************************************************
* Input Stream for Intel HEX File Decoding (friend function)
*******************************************************************************/
std::istream& operator>>(std::istream& dataIn, intelhex& ihLocal)
{
	// Create a string to store lines of Intel Hex info
	std::string ihLine;
	/* Create a string to store a single byte of Intel HEX info               */
	std::string ihByte;
	// Create an iterator for this variable
	std::string::iterator ihLineIterator;
	// Create a line counter
	unsigned long lineCounter = 0;
	// Variable to hold a single byte (two chars) of data
	unsigned char byteRead;
	// Variable to calculate the checksum for each line
	unsigned char intelHexChecksum;
	// Variable to hold the record length
	unsigned char recordLength;
	// Variable to hold the load offset
	unsigned long loadOffset;
	// Variables to hold the record type
	intelhexRecordType recordType;

	do
	{
		/* Clear the string before this next round                            */
		ihLine.erase();

		/* Clear the checksum before processing this line                     */
		intelHexChecksum = 0;

		/* Get a line of data                                                 */
		dataIn >> ihLine;

		/* If the line contained some data, process it                        */
		if (ihLine.length() > 0)
		{
			/* Increment line counter                                         */
			lineCounter++;

			/* Set string iterator to start of string                         */
			ihLineIterator = ihLine.begin();

			/* Check that we have a ':' record mark at the beginning          */
			if (*ihLineIterator != ':')
			{
				/* Add some warning code here                                 */
				std::string message;

				message = "Line without record mark ':' found @ line " +
					ihLocal.ulToString(lineCounter);

				ihLocal.addWarning(message);

				/* If this is the first line, let's simply give up. Chances   */
				/* are this is not an Intel HEX file at all                   */
				if (lineCounter == 1)
				{
					message = "Intel HEX File decode aborted; ':' missing in " \
						"first line.";
					ihLocal.addError(message);

					/* Erase ihLine content and break out of do...while loop  */
					ihLine.erase();
					break;
				}
			}
			else
			{
				/* Remove the record mark from the string as we don't need it */
				/* anymore                                                    */
				ihLine.erase(ihLineIterator);
			}

			/* Run through the whole line to check the checksum               */
			for (ihLineIterator = ihLine.begin();
				ihLineIterator != ihLine.end();
				/* Nothing - really! */)
			{
				/* Convert the line in pair of chars (making a single byte)   */
				/* into single bytes, and then add to the checksum variable.  */
				/* By adding all the bytes in a line together *including* the */
				/* checksum byte, we should get a result of '0' at the end.   */
				/* If not, there is a checksum error                          */
				ihByte.erase();

				ihByte = *ihLineIterator;
				++ihLineIterator;
				/* Just in case there are an odd number of chars in the       */
				/* just check we didn't reach the end of the string early     */
				if (ihLineIterator != ihLine.end())
				{
					ihByte += *ihLineIterator;
					++ihLineIterator;

					byteRead = ihLocal.stringToHex(ihByte);

					intelHexChecksum += byteRead;
				}
				else
				{
					std::string message;

					message = "Odd number of characters in line " +
						ihLocal.ulToString(lineCounter);

					ihLocal.addError(message);
				}
			}

			/* Make sure the checksum was ok                                  */
			if (intelHexChecksum == 0)
			{
				/* Reset iterator back to beginning of the line so we can now */
				/* decode it                                                  */
				ihLineIterator = ihLine.begin();

				/* Clear all the variables associated with decoding a line of */
				/* Intel HEX code.                                            */
				recordLength = 0;
				loadOffset = 0;

				/* Get the record length                                      */
				ihByte.erase();
				ihByte = *ihLineIterator;
				++ihLineIterator;
				ihByte += *ihLineIterator;
				++ihLineIterator;
				recordLength = ihLocal.stringToHex(ihByte);

				/* Get the load offset (2 bytes)                              */
				ihByte.erase();
				ihByte = *ihLineIterator;
				++ihLineIterator;
				ihByte += *ihLineIterator;
				++ihLineIterator;
				loadOffset =
					static_cast<unsigned long>(ihLocal.stringToHex(ihByte));
				loadOffset <<= 8;
				ihByte.erase();
				ihByte = *ihLineIterator;
				++ihLineIterator;
				ihByte += *ihLineIterator;
				++ihLineIterator;
				loadOffset +=
					static_cast<unsigned long>(ihLocal.stringToHex(ihByte));

				/* Get the record type                                        */
				ihByte.erase();
				ihByte = *ihLineIterator;
				++ihLineIterator;
				ihByte += *ihLineIterator;
				++ihLineIterator;
				recordType =
					static_cast<intelhexRecordType>(ihLocal.stringToHex(ihByte));

				/* Decode the INFO or DATA portion of the record              */
				switch (recordType)
				{
				case DATA_RECORD:
					ihLocal.decodeDataRecord(recordLength, loadOffset,
						ihLineIterator);
					if (ihLocal.verbose == true)
					{
						std::cout << "Data Record begining @ 0x" <<
							ihLocal.ulToHexString(loadOffset) << std::endl;
					}
					break;

				case END_OF_FILE_RECORD:
					/* Check that the EOF record wasn't already found. If */
					/* it was, generate appropriate error                 */
					if (ihLocal.foundEof == false)
					{
						ihLocal.foundEof = true;
					}
					else
					{
						std::string message;

						message = "Additional End Of File record @ line " +
							ihLocal.ulToString(lineCounter) +
							" found.";

						ihLocal.addError(message);
					}
					/* Generate error if there were                       */
					if (ihLocal.verbose == true)
					{
						std::cout << "End of File" << std::endl;
					}
					break;

				case EXTENDED_SEGMENT_ADDRESS:
					/* Make sure we have 2 bytes of data                  */
					if (recordLength == 2)
					{
						/* Extract the two bytes of the ESA               */
						unsigned long extSegAddress = 0;

						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						extSegAddress = static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));
						extSegAddress <<= 8;
						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						extSegAddress += static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));

						/* ESA is bits 4-19 of the segment base address   */
						/* (SBA), so shift left 4 bits                    */
						extSegAddress <<= 4;

						/* Update the SBA                                 */
						ihLocal.segmentBaseAddress = extSegAddress;
					}
					else
					{
						/* Note the error                                 */
						std::string message;

						message = "Extended Segment Address @ line " +
							ihLocal.ulToString(lineCounter) +
							" not 2 bytes as required.";

						ihLocal.addError(message);
					}
					if (ihLocal.verbose == true)
					{
						std::cout << "Ext. Seg. Address found: 0x" <<
							ihLocal.ulToHexString(ihLocal.segmentBaseAddress)
							<< std::endl;
					}

					break;

				case START_SEGMENT_ADDRESS:
					/* Make sure we have 4 bytes of data, and that no     */
					/* Start Segment Address has been found to date       */
					if (recordLength == 4 &&
						ihLocal.startSegmentAddress.exists == false)
					{
						/* Note that the Start Segment Address has been   */
						/* found.                                         */
						ihLocal.startSegmentAddress.exists = true;
						/* Clear the two registers, just in case          */
						ihLocal.startSegmentAddress.csRegister = 0;
						ihLocal.startSegmentAddress.ipRegister = 0;

						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						ihLocal.startSegmentAddress.csRegister =
							static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));
						ihLocal.startSegmentAddress.csRegister <<= 8;
						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						ihLocal.startSegmentAddress.csRegister +=
							static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));

						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						ihLocal.startSegmentAddress.ipRegister =
							static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));
						ihLocal.startSegmentAddress.ipRegister <<= 8;
						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						ihLocal.startSegmentAddress.ipRegister +=
							static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));
					}
					/* Note an error if the start seg. address already    */
					/* exists                                             */
					else if (ihLocal.startSegmentAddress.exists == true)
					{
						std::string message;

						message = "Start Segment Address record appears again @ line " +
							ihLocal.ulToString(lineCounter) +
							"; repeated record ignored.";

						ihLocal.addError(message);
					}
					/* Note an error if the start lin. address already    */
					/* exists as they should be mutually exclusive        */
					if (ihLocal.startLinearAddress.exists == true)
					{
						std::string message;

						message = "Start Segment Address record found @ line " +
							ihLocal.ulToString(lineCounter) +
							" but Start Linear Address already exists.";

						ihLocal.addError(message);
					}
					/* Note an error if the record lenght is not 4 as     */
					/* expected                                           */
					if (recordLength != 4)
					{
						std::string message;

						message = "Start Segment Address @ line " +
							ihLocal.ulToString(lineCounter) +
							" not 4 bytes as required.";

						ihLocal.addError(message);
					}
					if (ihLocal.verbose == true)
					{
						std::cout << "Start Seg. Address - CS 0x" <<
							ihLocal.ulToHexString(ihLocal.startSegmentAddress.csRegister) <<
							" IP 0x" <<
							ihLocal.ulToHexString(ihLocal.startSegmentAddress.ipRegister)
							<< std::endl;
					}
					break;

				case EXTENDED_LINEAR_ADDRESS:
					/* Make sure we have 2 bytes of data                  */
					if (recordLength == 2)
					{
						/* Extract the two bytes of the ELA               */
						unsigned long extLinAddress = 0;

						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						extLinAddress = static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));
						extLinAddress <<= 8;
						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						extLinAddress += static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));

						/* ELA is bits 16-31 of the segment base address  */
						/* (SBA), so shift left 16 bits                   */
						extLinAddress <<= 16;

						/* Update the SBA                                 */
						ihLocal.segmentBaseAddress = extLinAddress;
					}
					else
					{
						/* Note the error                                 */
						//cout << "Error in Ext. Lin. Address" << endl;

						std::string message;

						message = "Extended Linear Address @ line " +
							ihLocal.ulToString(lineCounter) +
							" not 2 bytes as required.";

						ihLocal.addError(message);
					}
					if (ihLocal.verbose == true)
					{
						std::cout << "Ext. Lin. Address 0x" <<
							ihLocal.ulToHexString(ihLocal.segmentBaseAddress)
							<< std::endl;
					}

					break;

				case START_LINEAR_ADDRESS:
					/* Make sure we have 4 bytes of data                  */
					if (recordLength == 4 &&
						ihLocal.startLinearAddress.exists == false)
					{
						/* Note that the linear start address has been    */
						/* found                                          */
						ihLocal.startLinearAddress.exists = true;

						/* Clear the EIP register                         */
						ihLocal.startLinearAddress.eipRegister = 0;

						/* Extract the four bytes of the SLA              */
						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						ihLocal.startLinearAddress.eipRegister =
							static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));
						ihLocal.startLinearAddress.eipRegister <<= 8;

						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						ihLocal.startLinearAddress.eipRegister +=
							static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));
						ihLocal.startLinearAddress.eipRegister <<= 8;

						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						ihLocal.startLinearAddress.eipRegister +=
							static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));
						ihLocal.startLinearAddress.eipRegister <<= 8;

						ihByte.erase();
						ihByte = *ihLineIterator;
						++ihLineIterator;
						ihByte += *ihLineIterator;
						++ihLineIterator;
						ihLocal.startLinearAddress.eipRegister +=
							static_cast<unsigned long>
							(ihLocal.stringToHex(ihByte));

					}
					/* Note an error if the start seg. address already    */
					/* exists                                             */
					else if (ihLocal.startLinearAddress.exists == true)
					{
						std::string message;

						message = "Start Linear Address record appears again @ line " +
							ihLocal.ulToString(lineCounter) +
							"; repeated record ignored.";

						ihLocal.addError(message);
					}
					/* Note an error if the start seg. address already    */
					/* exists as they should be mutually exclusive        */
					if (ihLocal.startSegmentAddress.exists == true)
					{
						std::string message;

						message = "Start Linear Address record found @ line " +
							ihLocal.ulToString(lineCounter) +
							" but Start Segment Address already exists.";

						ihLocal.addError(message);
					}
					/* Note an error if the record lenght is not 4 as     */
					/* expected                                           */
					if (recordLength != 4)
					{
						std::string message;

						message = "Start Linear Address @ line " +
							ihLocal.ulToString(lineCounter) +
							" not 4 bytes as required.";

						ihLocal.addError(message);
					}
					if (ihLocal.verbose == true)
					{
						std::cout << "Start Lin. Address - EIP 0x" <<
							ihLocal.ulToHexString(ihLocal.startLinearAddress.eipRegister)
							<< std::endl;
					}
					break;

				default:
					/* Handle the error here                              */
					if (ihLocal.verbose == true)
					{
						std::cout << "Unknown Record @ line " <<
							ihLocal.ulToString(lineCounter) << std::endl;
					}


					std::string message;

					message = "Unknown Intel HEX record @ line " +
						ihLocal.ulToString(lineCounter);

					ihLocal.addError(message);

					break;
				}
			}
			else
			{
				/* Note that the checksum contained an error                  */
				std::string message;

				message = "Checksum error @ line " +
					ihLocal.ulToString(lineCounter) +
					"; calculated 0x" +
					ihLocal.ucToHexString(intelHexChecksum - byteRead) +
					" expected 0x" +
					ihLocal.ucToHexString(byteRead);

				ihLocal.addError(message);
			}
		}
	} while (ihLine.length() > 0);

	if (ihLocal.verbose == true)
	{
		std::cout << "Decoded " << lineCounter << " lines from file." << std::endl;
	}

	return(dataIn);
}

/*******************************************************************************
* Output Stream for Intel HEX File Encoding (friend function)
*******************************************************************************/
std::ostream& operator<<(std::ostream& dataOut, intelhex& ihLocal)
{
	/* Stores the address offset needed by the linear/segment address records */
	unsigned long addressOffset;
	/* Iterator into the ihContent - where the addresses & data are stored    */
	std::map<unsigned long, unsigned char>::iterator ihIterator;
	/* Holds string that represents next record to be written                 */
	std::string thisRecord;
	/* Checksum calculation variable                                          */
	unsigned char checksum;

	thisRecord.clear();

	/* Check that there is some content to encode */
	if (ihLocal.ihContent.size() > 0)
	{
		/* Calculate the Linear/Segment address                               */
		ihIterator = ihLocal.ihContent.begin();
		addressOffset = (*ihIterator).first;
		checksum = 0;

		/* Construct the first record to define the segment base address      */
		if (ihLocal.segmentAddressMode == false)
		{
			unsigned char dataByte;

			addressOffset >>= 16;

			thisRecord = ":02000004";
			checksum = 0x02 + 0x04;

			dataByte = static_cast<unsigned char>(addressOffset & 0xFF);
			checksum += dataByte;
			thisRecord += ihLocal.ucToHexString(dataByte);

			dataByte = static_cast<unsigned char>((addressOffset >> 8) & 0xFF);
			checksum += dataByte;
			thisRecord += ihLocal.ucToHexString(dataByte);

			thisRecord += ihLocal.ucToHexString(0x00 - (checksum & 0xFF));
		}
		else
		{
			unsigned char dataByte;

			addressOffset >>= 4;

			thisRecord = ":02000002";
			checksum = 0x02 + 0x02;

			dataByte = static_cast<unsigned char>(addressOffset & 0xFF);
			checksum += dataByte;
			thisRecord += ihLocal.ucToHexString(dataByte);

			dataByte = static_cast<unsigned char>((addressOffset >> 8) & 0xFF);
			checksum += dataByte;
			thisRecord += ihLocal.ucToHexString(dataByte);

			thisRecord += ihLocal.ucToHexString(0x00 - (checksum & 0xFF));
		}

		/* Output the record                                                  */
		dataOut << thisRecord << std::endl;

		/* Now loop through all the available data and insert into file       */
		/* with maximum 16 bytes per line, and making sure to keep the        */
		/* segment base address up to date                                    */
		std::vector<unsigned char> recordData;
		unsigned long previousAddress;
		unsigned long currentAddress;
		unsigned long loadOffset;

		while (ihIterator != ihLocal.ihContent.end())
		{
			/* Check to see if we need to start a new linear/segment section  */
			loadOffset = (*ihIterator).first;

			/* If we are using the linear mode...                             */
			if (ihLocal.segmentAddressMode == false)
			{
				if ((loadOffset >> 16) != addressOffset)
				{
					unsigned char dataByte;

					thisRecord.clear();
					checksum = 0;

					addressOffset = loadOffset;
					addressOffset >>= 16;

					thisRecord = ":02000004";
					checksum = 0x02 + 0x04;

					dataByte = static_cast<unsigned char>(addressOffset & 0xFF);
					checksum += dataByte;
					thisRecord += ihLocal.ucToHexString(dataByte);

					dataByte = static_cast<unsigned char>((addressOffset >> 8) & 0xFF);
					checksum += dataByte;
					thisRecord += ihLocal.ucToHexString(dataByte);

					thisRecord += ihLocal.ucToHexString(0x00 - (checksum & 0xFF));

					/* Output the record                                      */
					dataOut << thisRecord << std::endl;
				}
			}
			/* ...otherwise assume segment mode                               */
			else
			{
				if ((loadOffset >> 4) != addressOffset)
				{
					unsigned char dataByte;

					thisRecord.clear();
					checksum = 0;

					addressOffset = loadOffset;
					addressOffset >>= 4;

					thisRecord = ":02000002";
					checksum = 0x02 + 0x02;

					dataByte = static_cast<unsigned char>(addressOffset & 0xFF);
					checksum += dataByte;
					thisRecord += ihLocal.ucToHexString(dataByte);

					dataByte = static_cast<unsigned char>((addressOffset >> 8) & 0xFF);
					checksum += dataByte;
					thisRecord += ihLocal.ucToHexString(dataByte);

					thisRecord += ihLocal.ucToHexString(0x00 - (checksum & 0xFF));

					/* Output the record                                      */
					dataOut << thisRecord << std::endl;
				}
			}

			/* Prepare for encoding next data record                          */
			thisRecord.clear();
			checksum = 0;
			recordData.clear();

			/* We need to check where the data actually starts, but only the  */
			/* bottom 16-bits; the other bits are in the segment/linear       */
			/* address record                                                 */
			loadOffset = (*ihIterator).first & 0xFFFF;

			/* Loop through and collect up to 16 bytes of data                */
			for (int x = 0; x < 16; x++)
			{
				currentAddress = (*ihIterator).first & 0xFFFF;

				recordData.push_back((*ihIterator).second);

				ihIterator++;

				/* Check that we haven't run out of data                      */
				if (ihIterator == ihLocal.ihContent.end())
				{
					break;
				}

				/* Check that the next address is consecutive                 */
				previousAddress = currentAddress;
				currentAddress = (*ihIterator).first & 0xFFFF;
				if (currentAddress != (previousAddress + 1))
				{
					break;
				}

				/* If we got here we have a consecutive address and can keep  */
				/* building up the data portion of the data record            */
			}

			/* Now we should have some data to encode; check first            */
			if (recordData.size() > 0)
			{
				std::vector<unsigned char>::iterator itData;
				unsigned char dataByte;

				/* Start building data record                                 */
				thisRecord = ":";

				/* Start with the RECLEN record length                        */
				dataByte = static_cast<unsigned char>(recordData.size());
				thisRecord += ihLocal.ucToHexString(dataByte);
				checksum += dataByte;

				/* Then the LOAD OFFSET                                       */
				dataByte = static_cast<unsigned char>((loadOffset >> 8) & 0xFF);
				thisRecord += ihLocal.ucToHexString(dataByte);
				checksum += dataByte;
				dataByte = static_cast<unsigned char>(loadOffset & 0xFF);
				thisRecord += ihLocal.ucToHexString(dataByte);
				checksum += dataByte;

				/* Then the RECTYP record type (no need to add to checksum -  */
				/* value is zero '00'                                         */
				thisRecord += "00";

				/* Now we add the data                                        */
				for (itData = recordData.begin(); itData != recordData.end(); itData++)
				{
					dataByte = (*itData);
					checksum += dataByte;
					thisRecord += ihLocal.ucToHexString(dataByte);
				}

				/* Last bit - add the checksum                                */
				thisRecord += ihLocal.ucToHexString(0x00 - (checksum & 0xFF));

				/* Now write the record                                       */
				dataOut << thisRecord << std::endl;
			}
		}
	}

	/* If there is a segment start address, output the data                   */
	if (ihLocal.startSegmentAddress.exists == true)
	{
		unsigned char dataByte;

		thisRecord.clear();
		checksum = 0;

		thisRecord = ":04000003";
		checksum = 0x04 + 0x03;

		dataByte = static_cast<unsigned char>((ihLocal.startSegmentAddress.csRegister >> 8) & 0xFF);
		checksum += dataByte;
		thisRecord += ihLocal.ucToHexString(dataByte);

		dataByte = static_cast<unsigned char>(ihLocal.startSegmentAddress.csRegister & 0xFF);
		checksum += dataByte;
		thisRecord += ihLocal.ucToHexString(dataByte);

		dataByte = static_cast<unsigned char>((ihLocal.startSegmentAddress.ipRegister >> 8) & 0xFF);
		checksum += dataByte;
		thisRecord += ihLocal.ucToHexString(dataByte);

		dataByte = static_cast<unsigned char>(ihLocal.startSegmentAddress.ipRegister & 0xFF);
		checksum += dataByte;
		thisRecord += ihLocal.ucToHexString(dataByte);


		/* Last bit - add the checksum                                        */
		thisRecord += ihLocal.ucToHexString(0x00 - (checksum & 0xFF));

		/* Now write the record                                               */
		dataOut << thisRecord << std::endl;
	}

	/* If there is a linear start address, output the data                    */
	if (ihLocal.startLinearAddress.exists == true)
	{
		unsigned char dataByte;

		thisRecord.clear();
		checksum = 0;

		thisRecord = ":04000005";
		checksum = 0x04 + 0x05;

		dataByte = static_cast<unsigned char>((ihLocal.startLinearAddress.eipRegister >> 24) & 0xFF);
		checksum += dataByte;
		thisRecord += ihLocal.ucToHexString(dataByte);

		dataByte = static_cast<unsigned char>((ihLocal.startLinearAddress.eipRegister >> 16) & 0xFF);
		checksum += dataByte;
		thisRecord += ihLocal.ucToHexString(dataByte);

		dataByte = static_cast<unsigned char>((ihLocal.startLinearAddress.eipRegister >> 8) & 0xFF);
		checksum += dataByte;
		thisRecord += ihLocal.ucToHexString(dataByte);

		dataByte = static_cast<unsigned char>(ihLocal.startLinearAddress.eipRegister & 0xFF);
		checksum += dataByte;
		thisRecord += ihLocal.ucToHexString(dataByte);


		/* Last bit - add the checksum                                        */
		thisRecord += ihLocal.ucToHexString(0x00 - (checksum & 0xFF));

		/* Now write the record                                               */
		dataOut << thisRecord << std::endl;
	}

	/* Whatever happened, we can always output the EOF record                 */
	dataOut << ":00000001FF" << std::endl;

	return (dataOut);
}

/*******************************************************************************
*
*                        INTEL HEX FILE CLASS MODULE END
*
*******************************************************************************/