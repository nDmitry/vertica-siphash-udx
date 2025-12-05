#include "Vertica.h"
#include "siphash.h"
#include <stdexcept>
#include <string>
#include <cstdlib>  // For strtoull
#include <stdint.h>
#include <cstring>  // For memset
#include <cstdio>   // For sprintf

using namespace Vertica;

/*
 * Vertica Scalar Function to calculate SipHash-2-4
 */
class SipHashFunction : public ScalarFunction {
public:
    virtual void processBlock(ServerInterface &srvInterface,
                              BlockReader &argReader,
                              BlockWriter &resWriter) {
        try {
            // Ensure exactly three arguments: k0, k1, and input string
            if (argReader.getNumCols() != 3) {
                vt_report_error(0, "Function expects 3 arguments: k0, k1, and a string to hash.");
            }

            // Read key components
            uint64_t key[2];
            key[0] = argReader.getIntRef(0); // k0
            key[1] = argReader.getIntRef(1); // k1

            // Loop through each row in the input
            do {
                // Read the input string
                const VString &input = argReader.getStringRef(2);

                // Handle NULL input
                if (input.isNull()) {
                    resWriter.getNumericRef().setNull();
                } else {
                    const std::string inputStr(input.data(), input.length());
                    uint8_t hashOutput[8]; // SipHash-2-4 produces an 8-byte hash

                    // Call the SipHash function
                    siphash(reinterpret_cast<const uint8_t *>(inputStr.data()), inputStr.size(), key, hashOutput, 8);

                    // Convert the 8-byte hash to a 64-bit unsigned integer
                    uint64_t hashValue = 0;
                    for (int i = 0; i < 8; i++) {
                        hashValue |= ((uint64_t)hashOutput[i]) << (8 * i);
                    }

                    // Cast hashValue to ifloat (long double)
                    ifloat hashIfloat = static_cast<ifloat>(hashValue);

                    // Set the result as VNumeric
                    VNumeric &hashNumeric = resWriter.getNumericRef();
                    hashNumeric.copy(hashIfloat, false); // Do not round
                }

                resWriter.next();
            } while (argReader.next());
        } catch (std::exception &e) {
            vt_report_error(0, "Exception while processing block: [%s]", e.what());
        }
    }
};

/*
 * Factory class for SipHashFunction
 */
class SipHashFunctionFactory : public ScalarFunctionFactory {
    virtual ScalarFunction *createScalarFunction(ServerInterface &interface) {
        return vt_createFuncObject<SipHashFunction>(interface.allocator);
    }

    virtual void getPrototype(ServerInterface &interface,
                              ColumnTypes &argTypes,
                              ColumnTypes &returnType) {
        argTypes.addInt();      // First key component k0 as INT
        argTypes.addInt();      // Second key component k1 as INT
        argTypes.addVarchar();  // Input string as VARCHAR
        returnType.addNumeric(); // Output type is NUMERIC
    }

    virtual void getReturnType(ServerInterface &srvInterface,
                               const SizedColumnTypes &inputTypes,
                               SizedColumnTypes &outputTypes) {
        outputTypes.addNumeric(20, 0, "siphash_result");
    }
};

// Register the factory class
RegisterFactory(SipHashFunctionFactory);

