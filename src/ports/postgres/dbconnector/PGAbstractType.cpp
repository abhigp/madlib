/* ----------------------------------------------------------------------- *//**
 *
 * @file PGAbstractType.cpp
 *
 *//* ----------------------------------------------------------------------- */

#include <dbconnector/PGCompatibility.hpp>
#include <dbconnector/PGAbstractType.hpp>
#include <dbconnector/PGArrayHandle.hpp>
#include <dbconnector/PGType.hpp>

extern "C" {
    #include <catalog/pg_type.h>
    #include <utils/array.h>
    #include <utils/typcache.h>
    #include <utils/lsyscache.h>
}

namespace madlib {

namespace dbconnector {

/**
 * @brief Convert postgres Datum into a ConcreteType object.
 */
AbstractTypeSPtr PGAbstractType::DatumToValue(bool inMemoryIsWritable,
    Oid inTypeID, Datum inDatum) const {
    
    bool isTuple = false;
    bool isArray = false;
    HeapTupleHeader pgTuple = NULL;
    ArrayType *pgArray = NULL;
    bool errorOccurred = false;
    
    PG_TRY(); {
        isTuple = type_is_rowtype(inTypeID);
        isArray = type_is_array(inTypeID);
        
        if (isTuple)
            pgTuple = DatumGetHeapTupleHeader(inDatum);
        else if (isArray)
            pgArray = DatumGetArrayTypeP(inDatum);
    } PG_CATCH(); {
        errorOccurred = true;
    } PG_END_TRY();
    
    BOOST_ASSERT_MSG(errorOccurred == false, "An exception occurred while "
        "converting a PostgreSQL datum to DBAL object.");

    // First check if datum is rowtype
    if (isTuple) {
        return AbstractTypeSPtr(new PGType<HeapTupleHeader>(pgTuple));
    } else if (isArray) {
        if (ARR_NDIM(pgArray) != 1)
            throw std::invalid_argument("Multidimensional arrays not yet supported");
        
        if (ARR_HASNULL(pgArray))
            throw std::invalid_argument("Arrays with NULLs not yet supported");
        
        switch (ARR_ELEMTYPE(pgArray)) {
            case FLOAT8OID: {
                MemHandleSPtr memoryHandle(
                    new PGArrayHandle(pgArray, AbstractHandle::kGlobal));
                
                if (inMemoryIsWritable) {
                    return AbstractTypeSPtr(
                        new ConcreteType<Array<double> >(
                            Array<double>(memoryHandle,
                                boost::extents[ ARR_DIMS(pgArray)[0] ])
                            )
                        );
                } else {
                    return AbstractTypeSPtr(
                        new ConcreteType<Array_const<double> >(
                            Array_const<double>(memoryHandle,
                                boost::extents[ ARR_DIMS(pgArray)[0] ])
                            )
                        );
                }
            }
            // FIXME: Default case
        }
    }

    switch (inTypeID) {
        case BOOLOID: return AbstractTypeSPtr(
            new ConcreteType<bool>( DatumGetBool(inDatum) ));
        case INT2OID: return AbstractTypeSPtr(
            new ConcreteType<int16_t>( DatumGetInt16(inDatum) ));
        case INT4OID: return AbstractTypeSPtr(
            new ConcreteType<int32_t>( DatumGetInt32(inDatum) ));
        case INT8OID: return AbstractTypeSPtr(
            new ConcreteType<int64_t>( DatumGetInt64(inDatum) ));
        case FLOAT4OID: return AbstractTypeSPtr(
            new ConcreteType<float>( DatumGetFloat4(inDatum) ));
        case FLOAT8OID: return AbstractTypeSPtr(
            new ConcreteType<double>( DatumGetFloat8(inDatum) ));
    }
    
    return AbstractTypeSPtr();
}

} // namespace dbconnector

} // namespace madlib
