#ifndef SMSHELPER_H
#define SMSHELPER_H

#include <QString>

namespace SMSHelper {
enum CountryCode {
    Australia,
    CzechRepublic,
    Other, // I only care about a few country codes
};

QString canonicalizePhoneNumber(const QString &phoneNumber);

bool isPhoneNumberMatchCanonicalized(const QString &canonicalPhone1, const QString &canonicalPhone2);

CountryCode determineCountryCode(const QString &canonicalNumber);

/**
 * See inline comments for how short codes are determined
 * All information from https://en.wikipedia.org/wiki/Short_code
 */
bool isShortCode(const QString &canonicalNumber, const CountryCode &country);

bool isAddressValid(const QString &address);

} // namespace SMSHelper

#endif // SMSHELPER_H
