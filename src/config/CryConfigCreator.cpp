#include "CryConfigCreator.h"
#include "CryCipher.h"

using cpputils::Console;
using cpputils::unique_ref;
using cpputils::RandomGenerator;
using std::string;
using std::vector;
using boost::optional;
using boost::none;

namespace cryfs {

    CryConfigCreator::CryConfigCreator(unique_ref<Console> console, RandomGenerator &encryptionKeyGenerator)
        :_console(std::move(console)), _encryptionKeyGenerator(encryptionKeyGenerator) {
    }

    CryConfig CryConfigCreator::create(const optional<string> &cipher) {
        CryConfig config;
        config.SetCipher(_generateCipher(cipher));
        config.SetEncryptionKey(_generateEncKey(config.Cipher()));
        config.SetRootBlob(_generateRootBlobKey());
        return config;
    }

    string CryConfigCreator::_generateCipher(const optional<string> &cipher) {
        if (cipher != none) {
            ASSERT(std::find(CryCiphers::supportedCipherNames().begin(), CryCiphers::supportedCipherNames().end(), *cipher) != CryCiphers::supportedCipherNames().end(), "Invalid cipher");
            return *cipher;
        } else {
            return _askCipher();
        }
    }

    string CryConfigCreator::_askCipher() {
        vector<string> ciphers = CryCiphers::supportedCipherNames();
        string cipherName = "";
        bool askAgain = true;
        while(askAgain) {
            int cipherIndex = _console->ask("Which block cipher do you want to use?", ciphers);
            cipherName = ciphers[cipherIndex];
            askAgain = !_showWarningForCipherAndReturnIfOk(cipherName);
        };
        return cipherName;
    }

    bool CryConfigCreator::_showWarningForCipherAndReturnIfOk(const string &cipherName) {
        auto warning = CryCiphers::find(cipherName).warning();
        if (warning == boost::none) {
            return true;
        }
        return _console->askYesNo(string() + (*warning) + " Do you want to take this cipher nevertheless?");
    }

    string CryConfigCreator::_generateEncKey(const std::string &cipher) {
        _console->print("\nGenerating secure encryption key...");
        auto key = CryCiphers::find(cipher).createKey(_encryptionKeyGenerator);
        _console->print("done\n");
        return key;
    }

    string CryConfigCreator::_generateRootBlobKey() {
        //An empty root blob entry will tell CryDevice to create a new root blob
        return "";
    }

}