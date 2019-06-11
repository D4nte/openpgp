/*
 Copyright 2019 SoloKeys Developers

 Licensed under the Apache License, Version 2.0, <LICENSE-APACHE or
 http://apache.org/licenses/LICENSE-2.0> or the MIT license <LICENSE-MIT or
 http://opensource.org/licenses/MIT>, at your option. This file may not be
 copied, modified, or distributed except according to those terms.
 */

#include <applets/openpgp/security.h>
#include "errors.h"
#include "applets/apduconst.h"
#include "solofactory.h"

namespace OpenPGP {

uint8_t Security::PasswdTryRemains(Password passwdId) {
	return pwstatus.PasswdTryRemains(passwdId);
}

Util::Error Security::DataObjectAccessCheck(
		uint16_t dataObjectID, bool writeAccess) {

	return Util::Error::NoError;
}

Util::Error Security::CommandAccessCheck(
		uint8_t cla, uint8_t ins, uint8_t p1, uint8_t p2) {

	// DataObjectAccessCheck
	if (ins == Applet::APDUcommands::GetData ||
		ins == Applet::APDUcommands::GetData2 ||
		ins == Applet::APDUcommands::PutData ||
		ins == Applet::APDUcommands::PutData2
		) {

		uint16_t object_id = (p1 << 8) + p2;

		auto err = DataObjectAccessCheck(
				object_id,
				ins == Applet::APDUcommands::PutData || ins == Applet::APDUcommands::PutData2);
		if (err != Util::Error::NoError)
			return err;
	}

	return Util::Error::NoError;
}

void Security::ClearAllAuth() {
	appletState.Clear();
}

void Security::Init() {
	Factory::SoloFactory &solo = Factory::SoloFactory::GetSoloFactory();
	File::FileSystem &filesystem = solo.GetFileSystem();

	ClearAllAuth();
	appletConfig.state = LifeCycleState::Init; // TODO: load
	pwstatus.Load(filesystem);
}

Util::Error Security::SetPasswd(Password passwdId, bstr passwords) {
	return Util::Error::NoError;
}

Util::Error Security::VerifyPasswd(Password passwdId, bstr data) {
	Factory::SoloFactory &solo = Factory::SoloFactory::GetSoloFactory();
	File::FileSystem &filesystem = solo.GetFileSystem();

	size_t min_length = PGPConst::PWMinLength(passwdId);
	size_t max_length = PGPConst::PWMaxLength(passwdId);

	uint8_t _passwd[max_length] = {0};
	bstr passwd(_passwd, 0, max_length);

	auto file_err = filesystem.ReadFile(File::AppletID::OpenPGP,
			(passwdId == Password::PW3) ? File::SecureFileID::PW3 : File::SecureFileID::PW1,
			File::Secure,
			passwd);
	if (file_err != Util::Error::NoError)
		return file_err;

	size_t passwd_length = passwd.length();

	if (passwd_length < min_length)
		return Util::Error::InternalError;

	// check allowing passwd check
	if (pwstatus.PasswdTryRemains(passwdId) == 0)
		return Util::Error::PasswordLocked;

	// check password
	if (data != passwd) {
		pwstatus.DecErrorCounter(passwdId);
		pwstatus.Save(filesystem);
		// TODO: maybe here need to add 0x6100 error
		return Util::Error::WrongPassword;
	}

	// OpenPGP v3.3.1 page 44
	SetAuth(passwdId);
	ResetPasswdTryRemains(passwdId);

	return Util::Error::NoError;
}

Util::Error Security::ResetPasswdTryRemains(Password passwdId) {
	Factory::SoloFactory &solo = Factory::SoloFactory::GetSoloFactory();
	File::FileSystem &filesystem = solo.GetFileSystem();

	pwstatus.PasswdSetRemains(passwdId, PGPConst::DefaultPWResetCounter);
	return pwstatus.Save(filesystem);
}

void Security::ClearAuth(Password passwdId) {
	switch (passwdId){
	case OpenPGP::Password::PW1:
		appletState.pw1Authenticated = false;
		break;
	case OpenPGP::Password::PW3:
		appletState.pw3Authenticated = false;
		break;
	case OpenPGP::Password::PSOCDS:
		appletState.cdsAuthenticated = false;
	default:
		break;
	}
}

void Security::SetAuth(Password passwdId) {
	switch (passwdId){
	case OpenPGP::Password::PW1:
		appletState.pw1Authenticated = true;
		break;
	case OpenPGP::Password::PW3:
		appletState.pw3Authenticated = true;
		break;
	case OpenPGP::Password::PSOCDS:
		appletState.cdsAuthenticated = true;
	default:
		break;
	}
}

bool Security::GetAuth(Password passwdId) {
	switch (passwdId){
	case OpenPGP::Password::PW1:
		return appletState.pw1Authenticated;
	case OpenPGP::Password::PW3:
		return appletState.pw3Authenticated;
	case OpenPGP::Password::PSOCDS:
		return appletState.cdsAuthenticated;
	default:
		return false;
	}
}

Util::Error Security::IncDSCounter() {
	Factory::SoloFactory &solo = Factory::SoloFactory::GetSoloFactory();
	File::FileSystem &filesystem = solo.GetFileSystem();

	DSCounter dscounter;
	auto cntrerr = dscounter.Load(filesystem);
	if (cntrerr != Util::Error::NoError)
		return cntrerr;

	dscounter.Counter++;

	cntrerr = dscounter.Save(filesystem);
	if (cntrerr != Util::Error::NoError)
		return cntrerr;

	return Util::Error::NoError;
}

} /* namespace OpenPGP */
