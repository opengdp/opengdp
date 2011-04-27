/*=========================================================================

 Program:   ORFEO Toolbox
 Language:  C++
 Date:      $Date$
 Version:   $Revision$


 Copyright (c) Centre National d'Etudes Spatiales. All rights reserved.
     See OTBCopyright.txt for details.


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


#include <stdio.h>
#include "otbCloudDetectionFunctor.h"
#include "otbCloudDetectionFilter.h"
// Software Guide : EndCodeSnippet

#include "itkExceptionObject.h"
#include "otbImage.h"
#include "otbVectorImage.h"
#include "otbImageFileReader.h"
#include "otbImageFileWriter.h"
#include "itkRescaleIntensityImageFilter.h"
#include "otbVectorRescaleIntensityImageFilter.h"
#include "otbMultiChannelExtractROI.h"


void usage(char *app) {

    fprintf(stderr, "Usage:\n");
    fprintf(stderr,"    %s <input File> <output File> <nbands>\n", app);
    fprintf(stderr,"        <band 1 Pixel Component>... <variance>\n");
    fprintf(stderr,"        <min Threshold 0-1> <maxThreshold 0-1>\n");


}


int main(int argc, char * argv[])
{

    if (argc < 7) {

        usage(argv[0]);
        return EXIT_FAILURE;
    }

    const unsigned int Dimension = 2;
    // Then we must decide what pixel type to use for the images. We choose to do
    // all the computations in double precision.

    typedef double InputPixelType;
    typedef double OutputPixelType;

    //  The images are defined using the pixel type and the
    //  dimension. Please note that the CloudDetectionFilter needs an
    //  VectorImage as input to handle multispectral images.

    typedef otb::VectorImage<InputPixelType, Dimension> VectorImageType;
    typedef VectorImageType::PixelType                  VectorPixelType;
    typedef otb::Image<OutputPixelType, Dimension>      OutputImageType;

    // We define the functor type that the filter will use. We use the
    // CloudDetectionFunctor.

    typedef otb::Functor::CloudDetectionFunctor<VectorPixelType,
    OutputPixelType>   FunctorType;

    // Now we can define the CloudDetectionFilter that takes a
    // multi-spectral image as input and produces a binary image.

    typedef otb::CloudDetectionFilter<VectorImageType, OutputImageType,
    FunctorType> CloudDetectionFilterType;

    //  An ImageFileReader class is also instantiated in order to read
    //  image data from a file. Then, an ImageFileWriter is instantiated
    //  in order to write the output image to a file.

    typedef otb::ImageFileReader<VectorImageType> ReaderType;
    typedef otb::ImageFileWriter<OutputImageType> WriterType;

    // The different filters composing our pipeline are created by invoking their
    // New() methods, assigning the results to smart pointers.

    ReaderType::Pointer               reader = ReaderType::New();
    CloudDetectionFilterType::Pointer cloudDetection =
        CloudDetectionFilterType::New();
    WriterType::Pointer writer = WriterType::New();

    /***** input file *****/

    reader->SetFileName(argv[1]);
    printf("infile = %s\n", argv[1]);
    cloudDetection->SetInput(reader->GetOutput());


    // The CloudDetectionFilter needs to have a reference
    // pixel corresponding to the spectral content likely to represent a
    // cloud. This is done by passing a pixel to the filter. Here we
    // suppose that the input image has four spectral bands.

    VectorPixelType referencePixel;
    referencePixel.SetSize(atoi(argv[3]));
    printf("nbands = %i\n", atoi(argv[3]));
    referencePixel.Fill(0.);

    int i;
    for (i = 0; i < atoi(argv[3]) ; i++) {
        printf ("referencePixel[%i] = %lg\n", i, atof(argv[4 + i]));
        referencePixel[i] = (atof(argv[4 + i]));
    }

    cloudDetection->SetReferencePixel(referencePixel);

    // We must also set the variance parameter of the filter and the
    // parameter of the gaussian functor.  The bigger the value, the
    // more tolerant the detector will be.

    cloudDetection->SetVariance(atof(argv[4 + i]));
    printf("variance = %lg\n", atof(argv[4 + i]));
    // The minimum and maximum thresholds are set to binarise the final result.
    // These values have to be between 0 and 1.

    cloudDetection->SetMinThreshold(atof(argv[5 + i]));
    printf("MinThreshold = %lg\n", atof(argv[5 + i]));

    cloudDetection->SetMaxThreshold(atof(argv[6 + i]));
    printf("MaxThreshold = %lg\n", atof(argv[6 + i]));

    writer->SetFileName(argv[2]);
    printf("outfile = %s\n", argv[2]);
    writer->SetInput(cloudDetection->GetOutput());
    writer->Update();


    return EXIT_SUCCESS;
}
