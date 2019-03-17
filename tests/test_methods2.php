<?php
ini_set('memory_limit', '256M');

$a = [[1,2,3], [4,5,6]];
$b = [[7,8,9], [10,11,12]];
$c = [[13,14], [15,16], [17,18]];
$afm = [[1,7,-2]];
$lrm = [[1,4,9]];

$vector = [[3,9,2]];
$vector2 = [[3],[9],[2]];

    $matA = Matrix::createFromData($a);

    $matAFM = Matrix::createFromData($afm);

    $matLrm = Matrix::createFromData($lrm);

    $matV = Matrix::createFromData($vector);
    $matV2 = Matrix::createFromData($vector2);

    $expected = [[0,0,0], [0,0,0]];
    $matZ = Matrix::zerosLike($matA);
    print("zerosLike");  
    print_r($expected);          
    print_r($matZ->toArray()) ;

    $expected = [[1,1,1], [1,1,1]];
    $matZ = Matrix::onesLike($matA);
    print("onesLike");  
    print_r($expected);          
    print_r($matZ->toArray()) ;

    
    // $afm = [[1,7,-2]];
    $expected = [[1,6,0.002*-2]];
    $matRL = $matAFM->reluFunc();
    print("reluFunc:");  
    print_r($expected);          
    print_r($matRL->toArray()) ;    

    // self->data[i * c + j] > 0)? 1:0.002
    $expected = [[1,1,0.002]];
    $matRL = $matAFM->reluDer();
    print("reluDer:");  
    print_r($expected);          
    print_r($matRL->toArray()) ;
    
    

    // tanhFunc
    $expected = [[tanh(1),tanh(7),tanh(-2)]];
    $matTan = $matAFM->tanhFunc();
    print("tanhFunc:");  
    print_r($expected);          
    print_r($matTan->toArray()) ;
    // tanhDer
    $expected  = [[0,-48,-3]];
    $matTan = $matAFM->tanhDer();
    print("tanhDer:");  
    print_r($expected);          
    print_r($matTan->toArray()) ;


    // adjustLr 1,4,9
    //$this->lr/(sqrt($value) + $this->betaParams['ϵ']);
    $expected = [[1,2/3,0.5]];
    $matLr = $matLrm->adjustLr(2,1);
    print("adjustLr:");  
    print_r($expected);          
    print_r($matLr->toArray()) ;    


    // expandRows  [3,9,2];
    //
    $expected = [[3,9,2],[3,9,2],[3,9,2]];
    $matV = $matV->expandRows(3);
    print("expandRows:");  
    print_r($expected);          
    print_r($matV->toArray()) ;    

        // expandColumns  $vector2 = [[3],[9],[2]];
    //
    $expected = [[3,3,3],[9,9,9],[2,2,2]];
    $matV2 = $matV2 ->expandColumns(3);
    print("expandColumns:");  
    print_r($expected);          
    print_r($matV2->toArray()) ;