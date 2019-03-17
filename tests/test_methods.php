<?php
ini_set('memory_limit', '256M');

$a = [[1,2,3], [4,5,6]];
$b = [[7,8,9], [10,11,12]];
$c = [[13,14], [15,16], [17,18]];
$vector = [3,9,2];

    $matA = Matrix::createFromData($a);
    $matX = $matA;
    $matB = Matrix::createFromData($b);

    $matC = Matrix::createFromData($c);

    //$expected = [[94,100],[229,244]];
    $matD = $matA->mul($matC);
    print("積：");        
    print_r($matD->toArray()) ;

    $expected = [[7,16,27],[40,55,72]];
    $matElemProduct = $matA->componentwiseProd($matB);
    print("要素同士の積：");    
    print_r($expected);
    print_r($matElemProduct->toArray());

    $expected = [[8,10,12],[14,16,18]];
    $matSum = $matA->plus($matB);
    print("和：");    
    print_r($expected);    
    print_r($matSum->toArray());

    $expected = [[-6,-6,-6],[-6,-6,-6]];
    $matDiff = $matA->minus($matB);
    print("差：");    
    print_r($expected);    
    print_r($matDiff->toArray());

    $expected = [[2,4,6],[8,10,12]];
    $matScalar = $matA->scale(doubleval(2));
    print("スカラー倍：");    
    print_r($expected);    
    print_r($matScalar->toArray());    

    $expected = [[1,4],[2,5],[3,6]];
    $matA = $matA->transpose();
    print("転置：");
    print_r($expected);    
    print_r($matA->toArray());

print("clone matrix:");

    // print($matA->get(0,0));
    print_r($matA->toArray());
    print_r($matX->toArray());    
