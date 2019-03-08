<?php
ini_set('memory_limit', '256M');

    $a = [[1,2,3], [4,5,6]];
    $matA = Matrix::createFromData($a);
    $b = [[7,8,9], [10,11,12]];
    $matB = Matrix::createFromData($b);
    $c = [[13,14], [15,16], [17,18]];
    $matC = Matrix::createFromData($c);

    $matProduct = $matA->mul($matB);
    $matElemProduct = $matA->componentwiseProd($matB);
    $matSum = $matA->plus($matB);
    $matDiff = $matA->minus($matB);
    $matScalar = $matA->scale(2);      
    
    $matTrans = $matB->transpose();


    echo "sum:";
    print_r($matSum);
    echo "diff:";
    print_r($matDiff);
    echo "element product:";
    print_r($matElemProduct);    
    echo "product:";
    print_r($matProduct->toArray());
    echo "scalar:";
    print_r($matScalar);
    echo "trans:";
    print_r($matTrans);

