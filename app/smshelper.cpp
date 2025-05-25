#include "smshelper.h"

#include <QRegularExpression>

namespace SMSHelper {
QString canonicalizePhoneNumber(const QString &phoneNumber)
{
    static const QRegularExpression leadingZeroes(QStringLiteral("^0*"));

    QString toReturn(phoneNumber);
    toReturn = toReturn.remove(QStringLiteral(" "));
    toReturn = toReturn.remove(QStringLiteral("-"));
    toReturn = toReturn.remove(QStringLiteral("("));
    toReturn = toReturn.remove(QStringLiteral(")"));
    toReturn = toReturn.remove(QStringLiteral("+"));
    toReturn = toReturn.remove(leadingZeroes);

    if (toReturn.isEmpty()) {
        // If we have stripped away everything, assume this is a special number (and already canonicalized)
        return phoneNumber;
    }
    return toReturn;
}

bool isPhoneNumberMatchCanonicalized(const QString &canonicalPhone1, const QString &canonicalPhone2)
{
    if (canonicalPhone1.isEmpty() || canonicalPhone2.isEmpty()) {
        // The empty string is not a valid phone number so does not match anything
        return false;
    }

    // To decide if a phone number matches:
    // 1. Are they similar lengths? If two numbers are very different, probably one is junk data and should be ignored
    // 2. Is one a superset of the other? Phone number digits get more specific the further towards the end of the string,
    //    so if one phone number ends with the other, it is probably just a more-complete version of the same thing
    const QString &longerNumber = canonicalPhone1.length() >= canonicalPhone2.length()
                                      ? canonicalPhone1
                                      : canonicalPhone2;
    const QString &shorterNumber = canonicalPhone1.length() < canonicalPhone2.length()
                                       ? canonicalPhone1
                                       : canonicalPhone2;

    const CountryCode &country = determineCountryCode(longerNumber);

    const bool shorterNumberIsShortCode = isShortCode(shorterNumber, country);
    const bool longerNumberIsShortCode = isShortCode(longerNumber, country);

    if (shorterNumberIsShortCode != longerNumberIsShortCode) {
        // If only one of the numbers is a short code, they clearly do not match
        return false;
    }

    bool matchingPhoneNumber = longerNumber.endsWith(shorterNumber);

    return matchingPhoneNumber;
}

CountryCode determineCountryCode(const QString &canonicalNumber)
{
    // This is going to fall apart if someone has not entered a country code into their contact book
    // or if Android decides it can't be bothered to report the country code, but probably we will
    // be fine anyway
    if (canonicalNumber.startsWith(QStringLiteral("41"))) {
        return CountryCode::Australia;
    }
    if (canonicalNumber.startsWith(QStringLiteral("420"))) {
        return CountryCode::CzechRepublic;
    }

    // The only countries I care about for the current implementation are Australia and CzechRepublic
    // If we need to deal with further countries, we should probably find a library
    return CountryCode::Other;
}

bool isShortCode(const QString &canonicalNumber, const CountryCode &country)
{
    // Regardless of which country this number belongs to, a number of length less than 6 is a "short code"
    if (canonicalNumber.length() <= 6) {
        return true;
    }
    if (country == CountryCode::Australia && canonicalNumber.length() == 8
        && canonicalNumber.startsWith(QStringLiteral("19"))) {
        return true;
    }
    if (country == CountryCode::CzechRepublic && canonicalNumber.length() <= 9) {
        // This entry of the Wikipedia article is fairly poorly written, so it is not clear whether a
        // short code with length 7 should start with a 9. Leave it like this for now, upgrade as
        // we get more information
        return true;
    }
    return false;
}

bool isAddressValid(const QString &address)
{
    QString canonicalizedNumber = canonicalizePhoneNumber(address);

    // This regular expression matches a wide range of international Phone numbers, minimum of 3 digits and maximum upto 15 digits
    static const QRegularExpression validNumberPattern(QStringLiteral("^(\\d{3,15})$"));
    if (validNumberPattern.match(canonicalizedNumber).hasMatch()) {
        return true;
    } else {
        static const QRegularExpression emailPattern(QStringLiteral("^[\\w\\.]*@[\\w\\.]*$"));
        if (emailPattern.match(address).hasMatch()) {
            return true;
        }
    }
    return false;
}

} // namespace SMSHelper
