#include "certificate_utils.hpp"
#include <https_socket_verify_utils.hpp>
#include <https_verifier_mac.hpp>
#include <icertificate_utils.hpp>
#include <logger.hpp>

#include <CoreFoundation/CoreFoundation.h>
#include <Security/Security.h>
#include <boost/asio/ssl.hpp>

#include <memory>
#include <string>

namespace https_socket_verify_utils
{
    bool HttpsVerifier::Verify(boost::asio::ssl::verify_context& ctx)
    {
        CFDataPtr certData;
        if (!ExtractCertificate(ctx, certData))
        {
            return false;
        }

        SecTrustPtr trust;
        if (!CreateTrustObject(certData, trust))
        {
            return false;
        }

        if (!EvaluateTrust(trust))
        {
            return false;
        }

        if (m_mode == "full")
        {
            SecCertificatePtr serverCert(m_utils->CreateCertificate(certData.get()));
            if (!serverCert || !ValidateHostname(serverCert))
            {
                return false;
            }
        }

        return true;
    }

    bool HttpsVerifier::ExtractCertificate(boost::asio::ssl::verify_context& ctx, CFDataPtr& certData)
    {
        STACK_OF(X509)* certChain = m_utils->GetCertChain(ctx.native_handle());
        if (!certChain || m_utils->GetCertificateCount(certChain) == 0)
        {
            LogError("No certificates in the chain.");
            return false;
        }

        X509* cert = m_utils->GetCertificateFromChain(certChain, 0);
        if (!cert)
        {
            LogError("The server certificate could not be obtained.");
            return false;
        }

        unsigned char* certRawData = nullptr;
        const int certLen = m_utils->EncodeCertificateToDER(cert, &certRawData);
        if (certLen <= 0)
        {
            LogError("Failed to encode certificate to DER.");
            return false;
        }

        certData = CFDataPtr(m_utils->CreateCFData(certRawData, certLen));
        OPENSSL_free(certRawData);

        return certData != nullptr;
    }

    bool HttpsVerifier::CreateTrustObject(const CFDataPtr& certData, SecTrustPtr& trust)
    {
        SecCertificatePtr serverCert(m_utils->CreateCertificate(certData.get()));

        if (!serverCert)
        {
            LogError("Failed to create SecCertificateRef.");
            return false;
        }

        const void* certArrayValues[] = {serverCert.get()};

        CFArrayPtr certArray(m_utils->CreateCertArray(certArrayValues, 1));
        SecPolicyPtr policy(m_utils->CreateSSLPolicy(true, ""));

        SecTrustRef rawTrust = nullptr;
        OSStatus status = m_utils->CreateTrustObject(certArray.get(), policy.get(), &rawTrust);

        if (status != errSecSuccess)
        {
            m_utils->ReleaseCFObject(rawTrust);
            LogError("Failed to create trust object.");
            return false;
        }

        trust.reset(rawTrust);
        return true;
    }

    bool HttpsVerifier::EvaluateTrust(const SecTrustPtr& trust)
    {
        CFErrorRef errorRef = nullptr;
        const bool trustResult = m_utils->EvaluateTrust(trust.get(), &errorRef);

        if (!trustResult && errorRef)
        {
            CFErrorPtr error(const_cast<__CFError*>(errorRef));

            CFStringPtr errorDesc(m_utils->CopyErrorDescription(errorRef));
            std::string errorString = m_utils->GetStringCFString(errorDesc.get());
            LogError("Trust evaluation failed: {}", errorString);
        }

        m_utils->ReleaseCFObject(errorRef);
        return trustResult;
    }

    bool HttpsVerifier::ValidateHostname(const SecCertificatePtr& serverCert)
    {
        CFStringPtr sanString(m_utils->CopySubjectSummary(serverCert.get()));
        if (!sanString)
        {
            LogError("Failed to retrieve SAN or CN for hostname validation.");
            return false;
        }

        bool hostnameMatches = false;
        std::string sanStringStr = m_utils->GetStringCFString(sanString.get());
        if (!sanStringStr.empty())
        {
            hostnameMatches = (m_host == sanStringStr);
        }

        if (!hostnameMatches)
        {
            LogError("The hostname '{}' does not match the certificate's SAN or CN '{}'.", m_host, sanStringStr);
        }

        return hostnameMatches;
    }

} // namespace https_socket_verify_utils
