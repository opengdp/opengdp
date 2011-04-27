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


    typedef otb::Image<double, 2>                   ImageType;
    typedef otb::VectorImage<double, 2>             VectorImageType;
    typedef otb::ImageFileReader<ImageType>         ReaderType;
    typedef otb::ImageFileReader<VectorImageType>   ReaderVectorType;

    /***** output types *****/
    
    typedef otb::VectorImage<unsigned char, 2> VectorByteImageType;
    typedef otb::VectorImage<unsigned short int, 2> VectorUint16ImageType;
    typedef otb::VectorImage<signed short int, 2> VectorInt16ImageType;
    typedef otb::VectorImage<unsigned int, 2> VectorUint32ImageType;
    typedef otb::VectorImage<signed int, 2> VectorInt32ImageType;

    /***** fusion filter types *****/

    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorByteImageType> FusionByteFilterType;
    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorUint16ImageType> FusionUint16FilterType;
    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorInt16ImageType> FusionInt16FilterType;
    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorUint32ImageType> FusionUint32FilterType;
    typedef otb::SimpleRcsPanSharpeningFusionImageFilter
        <ImageType, VectorImageType, VectorInt32ImageType> FusionInt32FilterType;

    /***** writer types *****/

    typedef otb::StreamingImageFileWriter<VectorByteImageType> WriterByteType;
    typedef otb::StreamingImageFileWriter<VectorUint16ImageType> WriterUint16Type;
    typedef otb::StreamingImageFileWriter<VectorInt16ImageType> WriterInt16Type;
    typedef otb::StreamingImageFileWriter<VectorUint32ImageType> WriterUint32Type;
    typedef otb::StreamingImageFileWriter<VectorInt32ImageType> WriterInt32Type;


void workByte(int argc, char* argv[]) {

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(argv[2]);
    
    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(argv[3]);

    FusionByteFilterType::Pointer fusion = FusionByteFilterType::New();
    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    WriterByteType::Pointer writer = WriterByteType::New();

    writer->SetFileName(argv[3]);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}

void workUint16(int argc, char* argv[]) {

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(argv[2]);
    
    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(argv[3]);

    FusionUint16FilterType::Pointer fusion = FusionUint16FilterType::New();
    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    WriterUint16Type::Pointer writer = WriterUint16Type::New();

    writer->SetFileName(argv[3]);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}

void workInt16(int argc, char* argv[]) {

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(argv[2]);
    
    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(argv[3]);

    FusionInt16FilterType::Pointer fusion = FusionInt16FilterType::New();
    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    WriterInt16Type::Pointer writer = WriterInt16Type::New();

    writer->SetFileName(argv[3]);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}

void workUint32(int argc, char* argv[]) {

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(argv[2]);
    
    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(argv[3]);

    FusionUint32FilterType::Pointer fusion = FusionUint32FilterType::New();
    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    WriterUint32Type::Pointer writer = WriterUint32Type::New();

    writer->SetFileName(argv[3]);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}

void workInt32(int argc, char* argv[]) {

    ReaderType::Pointer       readerPAN = ReaderType::New();
    readerPAN->SetFileName(argv[2]);
    
    ReaderVectorType::Pointer readerXS = ReaderVectorType::New();
    readerXS->SetFileName(argv[3]);

    FusionInt32FilterType::Pointer fusion = FusionInt32FilterType::New();
    fusion->SetPanInput(readerPAN->GetOutput());
    fusion->SetXsInput(readerXS->GetOutput());

    WriterInt32Type::Pointer writer = WriterInt32Type::New();

    writer->SetFileName(argv[3]);
    writer->SetInput(fusion->GetOutput());
    writer->Update();

}

int main(int argc, char* argv[]) {

    // Software Guide : EndCodeSnippet

    if (argc < 5)
    {
        std::cerr << "Missing Parameters " << std::endl;
        std::cerr << "Usage: " << argv[0];
        std::cerr <<
            " < byte || uint16 || int16 || uint32 || int32 > <inputPanchromatiqueImage> <inputMultiSpectralImage> <outputImage>"
            << std::endl;
        return EXIT_FAILURE;
    }    

    if (! strcasecmp(argv[1], "byte")) {
        workByte(argc, argv);
    }
    else if (! strcasecmp(argv[1], "uint16")) {
        workUint16(argc, argv);
    }
    else if (! strcasecmp(argv[1], "int16")) {
        workInt16(argc, argv);
    }
    else if (! strcasecmp(argv[1], "uint32")) {
        workUint32(argc, argv);
    }
    else if (! strcasecmp(argv[1], "int32")) {
        workInt32(argc, argv);
    }
    else {
        std::cerr << "Missing Parameters " << std::endl;
        std::cerr << "Usage: " << argv[0];
        std::cerr <<
            " < byte || uint16 || int16 || uint32 || int32 > <inputPanchromatiqueImage> <inputMultiSpectralImage> <outputImage>"
            << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
