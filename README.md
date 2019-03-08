# はじめに
PHPで行う行列計算を速くする為のPHP Extensionです。元のソースは、[dnishiyama85](https://github.com/dnishiyama85)/**PHPMatrix** です。Windows10上でコンパイルする際に、pthreadのコンパイルエラーが避けられなかったので、削除しました。また、OpenBLASも利用しません。


# コンパイルとインストール

Windows上でPHPのextension作成については、かなり長くなり、注意点もいくつかありますので、**extensionビルド方法.txt**ファイルを参照ください。

# サンプルコードの実行

testsフォルダにある**test_methods.php**で、いくつかのメソッドを実行できます。

```shell
# インストールしたPHPフォルダに移動
$ php.exe -d extension=c:\php\ext\php_matrix.dll test_methods.php
```
# LICENSE
This software is released under the MIT License, see LICENSE.

