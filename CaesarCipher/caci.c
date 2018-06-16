#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/*
    usage :caci -f filename [-k keynumber]
*/

/*******************************************************************
    for DEBUG
*******************************************************************/
#define _SWITCH_DEBUG_

#ifdef _SWITCH_DEBUG_
 #define DEBUG_PRINT(...)  printf(__VA_ARGS__)
 #define DEBUG_RETURN    return
#else
 #define DEBUG_PRINT(...)
 #define DEBUG_RETURN
#endif


/*******************************************************************
    型定義
*******************************************************************/
typedef unsigned char       uchar;
typedef signed char         schar;
typedef unsigned short int  ushort;
typedef unsigned long       ulong;


/*******************************************************************
    定数
*******************************************************************/
#define     MEMCMP_MATCH            (0)

#define     KEYNUMBER_DEFAULT       (3)         /* -K オプションを入れていない場合にこの値を使う */
#define     KEYNUMBER_SIZE          (2)         /* 2桁、0～99 */
#define     FILENAME_SIZE           (256)

#define     OUTPUTFILE_EXT          ".cc"       /* 出力ファイルに付与する拡張子 */



/*******************************************************************
    プロトタイプ
*******************************************************************/
uchar getCaesarCipher(uchar src, schar key);
uchar chgCase(uchar src);

/*******************************************************************
    main
*******************************************************************/
int main(int argc, char *argv[])
{
    /* 暗号化の際のキー */
    ushort  keynumber                           = KEYNUMBER_DEFAULT;
    uchar   szKeyNumber[KEYNUMBER_SIZE + 1]     = { 0 };

    /* ファイル関係 */
    FILE    *fpIn                               = NULL;
    uchar   szInFileName[FILENAME_SIZE + 1]     = { 0 };
    FILE    *fpOut                              = NULL;
    uchar   szOutFileName[FILENAME_SIZE + 1]    = { 0 };
    ushort  intOutFileNameLen                   = 0;
    ushort  intOutFileExtLen                    = 0;

    /* READした文字 */
    int     intC                                = '\0';
    uchar   ucC                                 = '\0';

    /* その他 */
    int     i;

    /*==============================================================*/
    /* コマンドライン解析                                           */
    /*==============================================================*/
    /* 0番目は実行ファイル名なので飛ばす */
    DEBUG_PRINT("argc:%d \n", argc);
    DEBUG_PRINT("argv[0]:%s \n", argv[0]);
    
    for(i=1; i<argc; i++) {
        /*----------------------------------------------------------*/
        /* F or f の場合                                            */
        /*----------------------------------------------------------*/
        if( ( MEMCMP_MATCH == memcmp(argv[i], "-F", 2) ) || 
            ( MEMCMP_MATCH == memcmp(argv[i], "-f", 2) ) ) {
            /* インデックス進めておく */
            i++;
            
            /* INPUTファイル名取得 */
            strncpy(szInFileName,  argv[i], FILENAME_SIZE);
            strncpy(szOutFileName, argv[i], FILENAME_SIZE);
            DEBUG_PRINT("-f:[%s] \n", szInFileName);
        }
        /*----------------------------------------------------------*/
        /* K or k の場合                                            */
        /*----------------------------------------------------------*/
        else if( ( MEMCMP_MATCH == memcmp(argv[i], "-K", 2) ) || 
                 ( MEMCMP_MATCH == memcmp(argv[i], "-k", 2) ) ) {
            /* インデックス進めておく */
            i++;
            
            /* キーナンバーの取得 */
            strncpy(szKeyNumber, argv[i], KEYNUMBER_SIZE);
            keynumber = atoi( szKeyNumber );
            DEBUG_PRINT("-f:[%s](%d) \n", szKeyNumber, keynumber);
        }
        /*----------------------------------------------------------*/
        /* 不明なオプションの場合                                   */
        /*----------------------------------------------------------*/
        else {
            printf("undefined option.(%s)", argv[i]);
            printf("usage : caci -F plainfilename [-K keynumber] \n");
            return;
        }
    }



    /*==============================================================*/
    /* 入力ファイルを元に、出力ファイルの名前を決める               */
    /*==============================================================*/
    intOutFileNameLen = strlen( szOutFileName );
    intOutFileExtLen  = strlen( OUTPUTFILE_EXT );
    
    if( (intOutFileNameLen + intOutFileExtLen) <= FILENAME_SIZE ) {
        /* 単に文字列を連結する */
        strncat( szOutFileName, OUTPUTFILE_EXT, intOutFileExtLen );
    }
    else {
        /* バッファをオーバーしてしまうため連結できない。そのため最後尾の必要な文字を上書きする */
        memcpy( &szOutFileName[intOutFileNameLen-KEYNUMBER_SIZE], OUTPUTFILE_EXT, intOutFileExtLen );
    }
    
    
    /*==============================================================*/
    /* １文字ずつ読み込んでファイル出力                             */
    /*==============================================================*/
    /*--------------------------------------------------------------*/
    /* OPEN（入力ファイル）                                         */
    /*--------------------------------------------------------------*/
    fpIn = fopen( szInFileName, "rb" );
    if( NULL == fpIn ) {
        printf("error : input file open error.(%s) \n", szInFileName);
        return -1;
    }
    /*--------------------------------------------------------------*/
    /* OPEN（出力ファイル）                                         */
    /*--------------------------------------------------------------*/
    fpOut = fopen( szOutFileName, "wb" );
    if( NULL == fpOut ) {
        printf("error : output file open error.(%s) \n", szOutFileName);
        return -1;
    }

    /*--------------------------------------------------------------*/
    /* READ ＆ 暗号化                                               */
    /*--------------------------------------------------------------*/
    /* キーナンバーから暗号用に使う値を算出する */
    while( (intC = fgetc( fpIn )) != EOF) {
        uchar ucFirstC = '\0';
        uchar ucLastC  = '\0';
        
        DEBUG_PRINT("-------- \n");
        DEBUG_PRINT("intC:%c \n", intC);
        
        ucC = intC & 0x00FF;
        DEBUG_PRINT("ucC:%c \n", ucC);
        
        
        /*--------------------------------------------------------------*/
        /* 文字の種類の判定                                             */
        /*--------------------------------------------------------------*/
        /* 小文字の場合 */
        if( ('a' <= ucC) && (ucC <= 'z') ) {
            ucFirstC = 'a';
            ucLastC  = 'z';
        }
        /* 大文字の場合 */
        else if( ('A' <= ucC) && (ucC <= 'Z') ) {
            ucFirstC = 'A';
            ucLastC  = 'Z';
        }
        /* 数字の場合 */
        else if( ('0' <= ucC) && (ucC <= '9') ) {
            ucFirstC = '0';
            ucLastC  = '9';
        }
        else {
            /* npp */
            /* そのまま出力する */
        }


        /*--------------------------------------------------------------*/
        /* （必要があれば）変換範囲内の文字に収める                     */
        /*--------------------------------------------------------------*/
        if(ucFirstC != '\0') {
            /*--------------------------------------------------------------*/
            /* 暗号化実施                                                   */
            /*--------------------------------------------------------------*/
            ucC = getCaesarCipher( ucC, keynumber );
            DEBUG_PRINT("ucC:%c \n", ucC);
            
            if(ucC > ucLastC) {
                ucC -= (ucLastC - ucFirstC + 1);
            }
            else if(ucC < ucFirstC) {
                ucC += (ucLastC - ucFirstC + 1);
            }
            else {
                /* nop */
            }
        }
        DEBUG_PRINT("ucC:%c \n", ucC);

        /*--------------------------------------------------------------*/
        /* 大文字小文字変換                                             */
        /*--------------------------------------------------------------*/
        ucC = chgCase(ucC);
        
        /*--------------------------------------------------------------*/
        /* ファイル出力                                                 */
        /*--------------------------------------------------------------*/
        DEBUG_PRINT("output:%c \n", ucC);
        fputc( ucC, fpOut );
    }

    /*--------------------------------------------------------------*/
    /* CLOSE                                                        */
    /*--------------------------------------------------------------*/
    fclose(fpIn);
    /*--------------------------------------------------------------*/
    /* CLOSE                                                        */
    /*--------------------------------------------------------------*/
    fclose(fpOut);

    return 0;
}


/********************************************************************/
/*  name      : 文字ずらし処理                                      */
/*  content   : 対象文字にキーを足したものを返す                    */
/*  parameter : (I) uchar 対象文字                                  */
/*              (I) schar キー                                      */
/*  return    : 対象文字にキーを足したもの                          */
/*  remarks   :                                                     */
/********************************************************************/
uchar getCaesarCipher(uchar src, schar key)
{
    DEBUG_PRINT("src:%d(%Xh) \n", src, src);
    DEBUG_PRINT("key:%d(%Xh) \n", key, key);
    DEBUG_PRINT("src + key:%d(%Xh) \n", src + key, src + key);
    return src + key;
}


/********************************************************************/
/*  name      : 大文字小文字処理                                    */
/*  content   : 大文字と小文字を変換する、英字以外はそのまま返す    */
/*  parameter : (I) uchar 対象文字                                  */
/*  return    : 対象文字を大文字or小文字に変換したもの              */
/*  remarks   :                                                     */
/********************************************************************/
uchar chgCase(uchar src)
{
    uchar diff = 'a' - 'A';
    
    /* 小文字の場合 */
    if( ('a' <= src) && (src <= 'z') ) {
        return src - diff;
    }
    /* 大文字の場合 */
    else if( ('A' <= src) && (src <= 'Z') ) {
        return src + diff;
    }
    /* その他 */
    else {
        return src;
    }
    
    /* ここに到達する事は無い */
    return src;
}

