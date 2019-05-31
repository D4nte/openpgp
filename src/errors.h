/*
  Copyright 2019 SoloKeys Developers

  Licensed under the Apache License, Version 2.0, <LICENSE-APACHE or
  http://apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT or
  http://opensource.org/licenses/MIT>, at your option. This file may not be
  copied, modified, or distributed except according to those terms.
 */

#ifndef SRC_ERRORS_H_
#define SRC_ERRORS_H_

namespace Util {
	enum Error {
		NoError,
		AppletNotSelected,
		AppletNotFound,
		WrongAPDUStructure,
		WrongAPDULength,
		WrongAPDUCLA,
		WrongAPDUINS,
		WrongAPDUP1P2,
		WrongAPDUDataLength,
		WrongCommand,

		DataNotFound,
		WrongPassword,

		FileNotFound,
		FileWriteError,

		InternalError,

		// this error links to the end of array
		lastError
	};

	static const char* const strError[Error::lastError + 1] = {
		"OK",
		"Applet not selected",
		"Applet not found",
		"Wrong APDU structure",
		"Wrong APDU length",
		"Wrong APDU CLA",
		"Wrong APDU INS",
		"Wrong APDU P1 or P2",
		"Wrong APDU data length",
		"Wrong command",

		"Data not found",
		"Wrong password",
		"File not found",
		"File write error",

		"Internal error",

		"n/a"};

	inline const char *GetStrError(Error err) {
		return strError[err];
	};
}



#endif /* SRC_ERRORS_H_ */
