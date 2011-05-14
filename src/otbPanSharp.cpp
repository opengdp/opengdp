/*=========================================================================

 Program:   ORFEO Toolbox
 Language:  C++
 Date:      $Date$
 Version:   $Revision$


 Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
     See OTBCopyright.txt for details.

     Some parts of this code are derived from ITK. See ITKCopyright.txt
     for details.


     This software is distributed WITHOUT ANY WARRANTY; without even
     the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
     PURPOSE.  See the above copyright notices for more information.

     =========================================================================*/

#if defined(_MSC_VER)
#pragma warning ( disable : 4786 )
#endif

#ifdef __BORLANDC__
#define ITK_LEAN_AND_MEAN
#endif

#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbStreamingImageFileWriter.h"
#include "otbSimpleRcsPanSharpeningFusionImageFilter.h"

#include "otbPrintableImageFilter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "otbImageToVectorImageCastFilter.h"
#include "otbVectorRescaleIntensityImageFilter.h"

#include "gdal.h"

void pansharp_byte(char *panfile, char *msfile, char *outfile) {
    std::cerr << panfile << std::endl;
    std::cerr << msfile << std::endl;
    std::cerr << outfile << std::endl;
    
    typedef otb::Image<double, 2>                   ImageType;
    typedef otb::VectorImage<double, 2>             VectorImageType;
    typedef otb::ImageFileReader<ImageType>         ReaderType;
    typedef otb::ImageFileReader<VectorImageType>   ReaderVectorType;
    typedef otb::VectorImage<GByte, 2> VectorIntImageType;

    /***** setup reader for pan band *****/

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(panfile);

    /***** setup reader for ms band *****/

    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(msfile);

    /***** setup the filter *****/

    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorIntImageType> FusionFilterType;

    FusionFilterType::Pointer fusion = FusionFilterType::New();

    /***** tie the inputs to the filter *****/

    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    /***** setup output *****/

    typedef otb::StreamingImageFileWriter<VectorIntImageType> WriterType;

    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(outfile);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}

void pansharp_uint16(char *panfile, char *msfile, char *outfile) {
    
    typedef otb::Image<double, 2>                   ImageType;
    typedef otb::VectorImage<double, 2>             VectorImageType;
    typedef otb::ImageFileReader<ImageType>         ReaderType;
    typedef otb::ImageFileReader<VectorImageType>   ReaderVectorType;
    typedef otb::VectorImage<GUInt16, 2> VectorIntImageType;

    /***** setup reader for pan band *****/

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(panfile);

    /***** setup reader for ms band *****/

    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(msfile);

    /***** setup the filter *****/

    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorIntImageType> FusionFilterType;

    FusionFilterType::Pointer fusion = FusionFilterType::New();

    /***** tie the inputs to the filter *****/

    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    /***** setup output *****/

    typedef otb::StreamingImageFileWriter<VectorIntImageType> WriterType;

    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(outfile);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}

void pansharp_int16(char *panfile, char *msfile, char *outfile) {
    
    typedef otb::Image<double, 2>                   ImageType;
    typedef otb::VectorImage<double, 2>             VectorImageType;
    typedef otb::ImageFileReader<ImageType>         ReaderType;
    typedef otb::ImageFileReader<VectorImageType>   ReaderVectorType;
    typedef otb::VectorImage<GInt16, 2> VectorIntImageType;

    /***** setup reader for pan band *****/

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(panfile);

    /***** setup reader for ms band *****/

    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(msfile);

    /***** setup the filter *****/

    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorIntImageType> FusionFilterType;

    FusionFilterType::Pointer fusion = FusionFilterType::New();

    /***** tie the inputs to the filter *****/

    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    /***** setup output *****/

    typedef otb::StreamingImageFileWriter<VectorIntImageType> WriterType;

    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(outfile);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}


void pansharp_uint32(char *panfile, char *msfile, char *outfile) {
    
    typedef otb::Image<double, 2>                   ImageType;
    typedef otb::VectorImage<double, 2>             VectorImageType;
    typedef otb::ImageFileReader<ImageType>         ReaderType;
    typedef otb::ImageFileReader<VectorImageType>   ReaderVectorType;
    typedef otb::VectorImage<GUInt32, 2> VectorIntImageType;

    /***** setup reader for pan band *****/

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(panfile);

    /***** setup reader for ms band *****/

    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(msfile);

    /***** setup the filter *****/

    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorIntImageType> FusionFilterType;

    FusionFilterType::Pointer fusion = FusionFilterType::New();

    /***** tie the inputs to the filter *****/

    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    /***** setup output *****/

    typedef otb::StreamingImageFileWriter<VectorIntImageType> WriterType;

    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(outfile);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}

void pansharp_int32(char *panfile, char *msfile, char *outfile) {
    
    typedef otb::Image<double, 2>                   ImageType;
    typedef otb::VectorImage<double, 2>             VectorImageType;
    typedef otb::ImageFileReader<ImageType>         ReaderType;
    typedef otb::ImageFileReader<VectorImageType>   ReaderVectorType;
    typedef otb::VectorImage<GInt32, 2> VectorIntImageType;

    /***** setup reader for pan band *****/

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(panfile);

    /***** setup reader for ms band *****/

    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(msfile);

    /***** setup the filter *****/

    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorIntImageType> FusionFilterType;

    FusionFilterType::Pointer fusion = FusionFilterType::New();

    /***** tie the inputs to the filter *****/

    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    /***** setup output *****/

    typedef otb::StreamingImageFileWriter<VectorIntImageType> WriterType;

    WriterType::Pointer writer = WriterType::New();

    writer->SetFileName(outfile);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}

void usage(char *arg0) {
    std::cerr << "Missing Parameters " << std::endl;
    std::cerr << "Usage: " << arg0;
    std::cerr <<
        " < byte || uint16 || int16 || uint32 || int32 > <inputPanchromatiqueImage> <inputMultiSpectralImage> <outputImage>"
         << std::endl;
    exit( EXIT_FAILURE);
}
    
int main(int argc, char* argv[]) {
    
    if (argc < 5)
    {
        usage(argv[0]);
    }    

    int type;
    for( type = 0; type < GDT_TypeCount; type++ ) {

        if ( GDALGetDataTypeName( (GDALDataType) type ) &&
             EQUAL(GDALGetDataTypeName((GDALDataType) type), argv[1])
           ) {
            break;
        }
    }

    switch (type) {
        
        /***** byte *****/

        case GDT_Byte:
            pansharp_byte(argv[2], argv[3], argv[4]);
            break;

        /***** uint16 *****/
        
        case GDT_UInt16:
            pansharp_uint16(argv[2], argv[3], argv[4]);
            break;

        /***** int16 *****/

        case GDT_Int16:
            pansharp_int16(argv[2], argv[3], argv[4]);
            break;

        /***** uint32 *****/

        case GDT_UInt32:
            pansharp_uint32(argv[2], argv[3], argv[4]);
            break;

        /***** int32 *****/

        case GDT_Int32:
            pansharp_int32(argv[2], argv[3], argv[4]);
            break;
        
        default:
            usage(argv[0]);
            break;
    }


    


    return EXIT_SUCCESS;
}
