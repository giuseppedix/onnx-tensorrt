/*
 * Copyright (c) 2021, NVIDIA CORPORATION. All rights reserved.
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

#ifndef NV_ONNX_PARSER_H
#define NV_ONNX_PARSER_H
#include <stdint.h>
#include "NvInfer.h"
#include <stddef.h>
#include <vector>

//!
//! \file NvOnnxParser.h
//!
//! This is the API for the ONNX Parser
//!

#define NV_ONNX_PARSER_MAJOR 0
#define NV_ONNX_PARSER_MINOR 1
#define NV_ONNX_PARSER_PATCH 0

static const int NV_ONNX_PARSER_VERSION = ((NV_ONNX_PARSER_MAJOR * 10000) + (NV_ONNX_PARSER_MINOR * 100) + NV_ONNX_PARSER_PATCH);

//! \typedef SubGraph_t
//!
//! \brief The data structure containing the parsing capability of
//! a set of nodes in an ONNX graph.
//!
using SubGraph_t = std::pair<std::vector<size_t>, bool>;

//! \typedef SubGraphCollection_t
//!
//! \brief The data structure containing all SubGraph_t partitioned
//! out of an ONNX graph.
//!
using SubGraphCollection_t = std::vector<SubGraph_t>;

class onnxTensorDescriptorV1;
//!
//! \namespace nvonnxparser
//!
//! \brief The TensorRT ONNX parser API namespace
//!
namespace nvonnxparser
{

template <typename T>
inline int32_t EnumMax();

/** \enum ErrorCode
 *
 * \brief the type of parser error
 */
enum class ErrorCode : int
{
    kSUCCESS = 0,
    kINTERNAL_ERROR = 1,
    kMEM_ALLOC_FAILED = 2,
    kMODEL_DESERIALIZE_FAILED = 3,
    kINVALID_VALUE = 4,
    kINVALID_GRAPH = 5,
    kINVALID_NODE = 6,
    kUNSUPPORTED_GRAPH = 7,
    kUNSUPPORTED_NODE = 8
};

template <>
inline int32_t EnumMax<ErrorCode>()
{
    return 9;
}

/** \class IParserError
 *
 * \brief an object containing information about an error
 */
class IParserError
{
public:
    /** \brief the error code
     */
    virtual ErrorCode code() const = 0;
    /** \brief description of the error
     */
    virtual const char* desc() const = 0;
    /** \brief source file in which the error occurred
     */
    virtual const char* file() const = 0;
    /** \brief source line at which the error occurred
     */
    virtual int line() const = 0;
    /** \brief source function in which the error occurred
     */
    virtual const char* func() const = 0;
    /** \brief index of the ONNX model node in which the error occurred
     */
    virtual int node() const = 0;

protected:
    virtual ~IParserError() {}
};

/** \class IParser
 *
 * \brief an object for parsing ONNX models into a TensorRT network definition
 */
class IParser
{
public:
    /** \brief Parse a serialized ONNX model into the TensorRT network.
     *         This method has very limited diagnostic. If parsing the serialized model
     *         fails for any reason (e.g. unsupported IR version, unsupported opset, etc.)
     *         it the user responsibility to intercept and report the error.
     *         To obtain a better diagnostic, use the parseFromFile method below.
     *
     * \param serialized_onnx_model Pointer to the serialized ONNX model
     * \param serialized_onnx_model_size Size of the serialized ONNX model
     *        in bytes
     * \param model_path Absolute path to the model file for loading external weights if required
     * \return true if the model was parsed successfully
     * \see getNbErrors() getError()
     */
    virtual bool parse(void const* serialized_onnx_model,
                       size_t serialized_onnx_model_size,
                       const char* model_path = nullptr)
        = 0;

    /** \brief Parse an onnx model file, can be a binary protobuf or a text onnx model
     *         calls parse method inside.
     *
     * \param File name
     * \param Verbosity Level
     *
     * \return true if the model was parsed successfully
     *
     */
    virtual bool parseFromFile(const char* onnxModelFile, int verbosity) = 0;

    /** \brief Check whether TensorRT supports a particular ONNX model
     *
     * \param serialized_onnx_model Pointer to the serialized ONNX model
     * \param serialized_onnx_model_size Size of the serialized ONNX model
     *        in bytes
     * \param sub_graph_collection Container to hold supported subgraphs
     * \param model_path Absolute path to the model file for loading external weights if required
     * \return true if the model is supported
     */
    virtual bool supportsModel(void const* serialized_onnx_model,
                               size_t serialized_onnx_model_size,
                               SubGraphCollection_t& sub_graph_collection,
                               const char* model_path = nullptr)
        = 0;

    /** \brief Parse a serialized ONNX model into the TensorRT network
     * with consideration of user provided weights
     *
     * \param serialized_onnx_model Pointer to the serialized ONNX model
     * \param serialized_onnx_model_size Size of the serialized ONNX model
     *        in bytes
     * \param weight_count number of user provided weights
     * \param weight_descriptors pointer to user provided weight array
     * \return true if the model was parsed successfully
     * \see getNbErrors() getError()
     */
    virtual bool parseWithWeightDescriptors(
        void const* serialized_onnx_model, size_t serialized_onnx_model_size,
        uint32_t weight_count,
        onnxTensorDescriptorV1 const* weight_descriptors)
        = 0;

    /** \brief Returns whether the specified operator may be supported by the
     *         parser.
     *
     * Note that a result of true does not guarantee that the operator will be
     * supported in all cases (i.e., this function may return false-positives).
     *
     * \param op_name The name of the ONNX operator to check for support
     */
    virtual bool supportsOperator(const char* op_name) const = 0;
    /** \brief destroy this object
     */
    virtual void destroy() = 0;
    /** \brief Get the number of errors that occurred during prior calls to
     *         \p parse
     *
     * \see getError() clearErrors() IParserError
     */
    virtual int getNbErrors() const = 0;
    /** \brief Get an error that occurred during prior calls to \p parse
     *
     * \see getNbErrors() clearErrors() IParserError
     */
    virtual IParserError const* getError(int index) const = 0;
    /** \brief Clear errors from prior calls to \p parse
     *
     * \see getNbErrors() getError() IParserError
     */
    virtual void clearErrors() = 0;

    /** \brief Get description of all ONNX weights that can be refitted.
     * 
     * \param weightsNames Where to write the weight names to
     * \param layerNames Where to write the layer names to
     * \param roles Where to write the roles to
     *
     * \return The number of weights from the ONNX model that can be refitted
     *
     * If weightNames or layerNames != nullptr, each written pointer points to a string owned by
     * the parser, and becomes invalid when the parser is destroyed
     *
     * If the same weight is used in multiple TRT layers it will be represented as a new
     * entry in weightNames with name <weightName>_x, with x being the number of times the weight
     * has been used before the current layer
     */
    virtual int getRefitMap(const char** weightNames, const char** layerNames, nvinfer1::WeightsRole* roles) = 0;

protected:
    virtual ~IParser() {}
};

} // namespace nvonnxparser

extern "C" TENSORRTAPI void* createNvOnnxParser_INTERNAL(void* network, void* logger, int version);
extern "C" TENSORRTAPI int getNvOnnxParserVersion();

namespace nvonnxparser
{

#ifdef SWIG
inline IParser* createParser(nvinfer1::INetworkDefinition* network,
                             nvinfer1::ILogger* logger)
{
    return static_cast<IParser*>(
        createNvOnnxParser_INTERNAL(network, logger, NV_ONNX_PARSER_VERSION));
}
#endif // SWIG

namespace
{

/** \brief Create a new parser object
 *
 * \param network The network definition that the parser will write to
 * \param logger The logger to use
 * \return a new parser object or NULL if an error occurred
 * \see IParser
 */
#ifdef _MSC_VER
TENSORRTAPI IParser* createParser(nvinfer1::INetworkDefinition& network,
                                  nvinfer1::ILogger& logger)
#else
inline IParser* createParser(nvinfer1::INetworkDefinition& network,
                             nvinfer1::ILogger& logger)
#endif
{
    return static_cast<IParser*>(
        createNvOnnxParser_INTERNAL(&network, &logger, NV_ONNX_PARSER_VERSION));
}

} // namespace

} // namespace nvonnxparser

#endif // NV_ONNX_PARSER_H
