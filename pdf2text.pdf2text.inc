<?php
// $Id: pdf2text.pdf2text.inc,v 1.1 2010/06/04 07:32:18 saubhagya Exp $

include("initialize.pdf2text.inc");

// Global variables
$_infile;
$_maxHeight;
$_textAccordingX;
$_document;
$_pdfDocToUni;
$_winToPdf;
$_uniCode;
$_trans;

/**
 * Position the content according to its x values in given line.
 * @param $textContainers
 * @param $graphicsContainers
 * @param $mediaBox_x
 * @param $mediaBox_y
 * @param $fonts
 * @param $xobjects
 * @param $rotation
 */
function _pdf2text_get_texts($textContainers, $graphicsContainers, $mediaBox_x, $mediaBox_y, $fonts, $xobjects, $rotation) {
  global $_maxHeight;
  global $_textAccordingX;
  $_textAccordingX = array();
  $flag;
  $CTM[0][0] = 1;
  $CTM[0][1] = 0;
  $CTM[1][0] = 0;
  $CTM[1][1] = 1;
  $CTM[2][0] = 0;
  $CTM[2][1] = 0;
  $CG = $CTM;
  $Tf = 1;
  $Tz = 1;
  $order = 1;
  $pointer = -1;
  $graphicStack = array();

  for ($j = 0; $j < count($textContainers); $j++) {
    $Tm[0][0] = 1;
    $Tm[0][1] = 0;
    $Tm[1][0] = 0;
    $Tm[1][1] = 1;
    $Tm[2][0] = 0;
    $Tm[2][1] = 0;
    $graphics = $graphicsContainers[$j];
    do {
      $gType = 0;
      $pos = strpos($graphics, "q");
      if ($pos !== FALSE) {
        $gType = 1;
        $minPos = $pos;
      }
      $pos = strpos($graphics, "Q");
      if ($pos !== FALSE && (!$gType || $pos < $minPos)) {
        $gType = 2;
        $minPos = $pos;
      }
      $pos = strpos($graphics, "cm");
      if ($pos !== FALSE && (!$gType || $pos < $minPos)) {
        $gType = 3;
        $minPos = $pos;
      }
      $pos = strpos($graphics, "Do");
      if ($pos !== FALSE && (!$gType || $pos < $minPos)) {
        $gType = 4;
        $minPos = $pos;
      }
      if ($gType) {
        if ($gType == 1) {
          $graphicStack[++$pointer] = $CG;
          $graphics = substr($graphics, $minPos + 1);
        }
        elseif ($gType == 2) {
          if ($pointer > -1) {
            $CG = $graphicStack[$pointer--];
          }
          else {
            $CG = $CTM;
          }
          $graphics = substr($graphics, $minPos + 1);
        }
        elseif ($gType == 3) {
          preg_match_all("#([\-0-9\.]+)\s*([\-0-9\.]+)\s*([\-0-9\.]+)\s*([\-0-9\.]+)\s*([\-0-9\.]+)\s*([\-0-9\.]+)\s*cm#sm", $graphics, $temp);
          $gp[0] = $temp[1][0];
          $gp[1] = $temp[2][0];
          $gp[2] = $temp[3][0];
          $gp[3] = $temp[4][0];
          $gp[4] = $temp[5][0];
          $gp[5] = $temp[6][0];

          $temp = $CG;

          $CG[0][0] = $gp[0] * $temp[0][0] + $gp[1] * $temp[1][0];
          $CG[0][1] = $gp[0] * $temp[0][1] + $gp[1] * $temp[1][1];
          $CG[1][0] = $gp[2] * $temp[0][0] + $gp[3] * $temp[1][0];
          $CG[1][1] = $gp[2] * $temp[0][1] + $gp[3] * $temp[1][1];
          $CG[2][0] = $gp[4] * $temp[0][0] + $gp[5] * $temp[1][0] + $temp[2][0];
          $CG[2][1] = $gp[4] * $temp[0][1] + $gp[5] * $temp[1][1] + $temp[2][1];
          $graphics = substr($graphics, $minPos + 2);
        }
        else {
          if (preg_match("#([a-zA-Z0-9]+)\s*Do#smU", $graphics, $xobject)) {
            $xobject = $xobject[1];
            $xo = "";
            $xo = _pdf2text_search_object($xobjects[$xobject]);
            if ($xo != "" && strpos($xo, "Form")) {
              $xo = _pdf2text_get_stream($xo);
              _pdf2text_render_content(_pdf2text_add_content($xo, $mediaBox_x, $mediaBox_y, $fonts, $xobjects, $rotation));
            }
          }
          $graphics = substr($graphics, $minPos + 2);
        }
      }
    } while ($gType);
    $str = $textContainers[$j];
    do {
      $type = 0;
      $text = "";
      if (preg_match("#(.*\))\s*Tj#smU", $str, $part1)) {
        $part1 = @ $part1[1];
        $part = $part1;
        $len = strlen($part1);
        $type = 1;
      }

      if (preg_match("#(.*\>)\]\s*TJ#smU", $str, $part6)) {
        $part6 = @$part6[1];
        if (!$type || strlen($part6) < $len) {
          $len = strlen($part6);
          $part = $part6;
          $type = 6;
        }
      }

      if (preg_match("#(.*)\]\s*TJ#smU", $str, $part2)) {
        $part2 = @ $part2[1];
        if (!$type || strlen($part2) < $len) {
          $part = $part2;
          $len = strlen($part2);
          $type = 2;
        }
      }

      if (preg_match("#(.*\))\s*\'#smU", $str, $part3)) {
        $part3 = @ $part3[1];
        if (!$type || strlen($part3) < $len) {
          $part = $part3;
          $len = strlen($part3);
          $type = 3;
        }
      }

      if (preg_match("#(.*\))\s*\"#smU", $str , $part4)) {
        $part4 = @ $part4[1];
        if (!$type || strlen($part4) < $len) {
          $part = $part4;
          $len = strlen($part4);
          $type = 4;
        }
      }

      if (preg_match("#(.*\>)\s*Tj#smU", $str, $part5)) {
        $part5 = @$part5[1];
        if (!$type || strlen($part5) < $len) {
          $part = $part5;
          $len = strlen($part5);
          $type = 5;
        }
      }

      if ($type) {
        $o = "";
        if (preg_match("#([0-9a-zA-Z_]+) ([\-0-9\.]+) Tf#sm", $part, $_Tf)) {
          $Tf = $_Tf[2];
          $fontType = $_Tf[1];
        }
        if ($fontType != "") {
          $o = _pdf2text_search_object($fonts[$fontType]);
        }

        if ($type == 2) {
          preg_match("#\[(.*)#sm", $part, $text);
          $text = @$text[1];
          _pdf2text_convert_to_unicode($text, $o, 1);
          $pos = strpos($str, "TJ");
          $str = substr($str, $pos + 3);
          $flag = 1;
        }

        elseif ($type == 1) {
          preg_match("#(\(.*\))#sm", $part, $text);
          $text = @$text[1];
          _pdf2text_convert_to_unicode($text, $o, 0);
          $pos = strpos($str, "Tj");
          $str = substr($str, $pos+3);
          $flag = 0;
        }

        elseif ($type == 3) {
          preg_match("#(\(.*\))#sm", $part, $text);
          $text = @$text[1];
          _pdf2text_convert_to_unicode($text, $o, 0);
          $pos = strpos($str, "'");
          $str = substr($str, $pos+1);
          $flag = 0;
        }

        elseif ($type == 4) {
          preg_match("#(\(.*\))#sm", $part, $text);
          $text = @$text[1];
          _pdf2text_convert_to_unicode($text, $o, 0);
          $pos = strpos($str, "\"");
          $str = substr($str, $pos+1);
          $flag = 0;
        }

        elseif ($type == 5) {
          if (preg_match("#ToUnicode\s*([0-9]+)\s*0\s*R#sm", $o, $ucObjectNo)) {
            $ucObjectNo = $ucObjectNo[1];
            $ucObject = _pdf2text_search_object($ucObjectNo);
            $stream = _pdf2text_get_stream($ucObject);
            _pdf2text_get_char_transformations($transformations, $stream);

            preg_match("#\<(.*)\>#sm", $part, $hexPart);
            $hexPart = $hexPart[1];
            if (strlen($hexPart)%2) {
              $hexPart .= "0";
            }
            $hexS = str_split($hexPart, 2);
            $text = "(";
            foreach ($hexS as $hex) {
              $chex = str_pad($hex, 4, "0");
              if (isset($transformations[$chex])) {
                $text .= html_entity_decode("&#x". $transformations[$chex] .";");
              }
            }
            $text .= ")";
          }
          $pos = strpos($str, "Tj");
          $str = substr($str, $pos+3);
          $flag = 0;
        }

        elseif ($type == 6) {
          if (preg_match("#ToUnicode\s*([0-9]+)\s*0\s*R#sm", $o, $ucObjectNo)) {
            $ucObjectNo = $ucObjectNo[1];
            $ucObject = _pdf2text_search_object($ucObjectNo);
            $stream = _pdf2text_get_stream($ucObject);
            _pdf2text_get_char_transformations($transformations, $stream);

            preg_match("#\[(.*)#sm", $part, $hexPartStr);
            $hexPartStr = $hexPartStr[1];
            if (preg_match("#\<(.*)\>#smU", $hexPartStr, $hexPart)) {
              $hexPart = @$hexPart[1];
              $pos = strpos($hexPartStr, ">");
            }
            if (strlen($hexPart)%2) {
              $hexPart .= "0";
            }
            $hexS = str_split($hexPart, 2);
            $text = "(";
            foreach ($hexS as $hex) {
              $chex = str_pad($hex, 4, "0");
              if (isset($transformations[$chex])) {
                $text .= html_entity_decode("&#x". $transformations[$chex] .";");
              }
            }
            $text .= ")";
            $hexPartStr = substr($hexPartStr, $pos+1);
            if ($pos) {
              preg_match_all("#([\-0-9\.]+)\s*\<(.*)\>#smU", $hexPartStr, $rest);
              $digParts = @$rest[1];
              $hexParts = @$rest[2];
              for ($i = 0; $i < count($digParts); $i++) {
                $text .= $digParts[$i];
                if (strlen($hexParts[$i])%2) {
                  $hexParts[$i] .= "0";
                }
                $hexS = str_split($hexParts[$i], 2);
                $text .= "(";
                foreach ($hexS as $hex) {
                  $chex = str_pad($hex, 4, "0");
                  if (isset($transformations[$chex])) {
                    $text .= html_entity_decode("&#x". $transformations[$chex] .";");
                  }
                }
                $text .= ")";
              }
            }
          }
          $pos = strpos($str, "TJ");
          $str = substr($str, $pos+3);
          $flag = 1;
        }

        if (preg_match("#([\-0-9\.]+)\s+([\-0-9\.]+)\s+([\-0-9\.]+)\s+([\-0-9\.]+)\s+([\-0-9\.]+)\s+([\-0-9\.]+)\s*Tm#sm", $part, $Temp)) {
          if ($Temp[1]) {
            $Tm[0][0] = $Temp[1];
            $Tm[0][1] = $Temp[2];
            $Tm[1][0] = $Temp[3];
            $Tm[1][1] = $Temp[4];
            $Tm[2][0] = $Temp[5];
            $Tm[2][1] = $Temp[6];
            $order = 1;
          }
          else {
            $Tm[0][0] = $Temp[2];
            $Tm[0][1] = $Temp[1];
            $Tm[1][0] = $Temp[4];
            $Tm[1][1] = $Temp[3];
            $Tm[2][0] = $Temp[6];
            $Tm[2][1] = $Temp[5];
            $order = 0;
          }
        }
        if ($Trm[0][0] < 0) {
          $Trm[0][0] *= -1;
        }
        if ($Trm[1][1] < 0) {
          $Trm[1][1] *= -1;
        }
        if ($Trm[2][0] < 0) {
          $Trm[2][0] *= -1;
        }
        if ($Trm[2][1] < 0) {
          $Trm[2][1] *= -1;
        }
        if (preg_match("#([\-0-9\.]+)\s*([\-0-9\.]+)\s*TD#sm", $part, $TD)) {
          if (!$order) {
            $a = $TD[1];
            $TD[1] = $TD[0];
            $TD[0] = $a;
          }
          $TL = -1 * $TD[2];
          $Tm[2][0] += $Tm[0][0] * $TD[1] + $Tm[1][0] * $TD[2];
          $Tm[2][1] += $Tm[0][1] * $TD[1] + $Tm[1][1] * $TD[2];
        }
        if (preg_match("#([\-0-9\.]+)\s*([\-0-9\.]+)\s*Td#sm", $part, $Td)) {
          if (!$order) {
            $a = $Td[1];
            $Td[1] = $Td[0];
            $Td[0] = $a;
          }
          $Tm[2][0] += $Tm[0][0] * $Td[1] + $Tm[1][0] * $Td[2];
          $Tm[2][1] += $Tm[0][1] * $Td[1] + $Tm[1][1] * $Td[2];
        }
        if (preg_match("#([\-0-9\.]+)\s*TL#sm", $part, $_TL)) {
          $TL = $_TL[1];
        }
        if (preg_match_all("#T\*(.*)#sm", $part, $Tst)) {
          $Tst = $Tst[1];
          $Tm[2][0] =  $Tm[2][0] - count($Tst) * $TL * $Tm[1][0];
          $Tm[2][1] =  $Tm[2][1] - count($Tst) * $TL * $Tm[1][1];
        }
        if ($type == 3 || $type == 4) {
          $Tm[2][0] =  $Tm[2][0] - $TL * $Tm[1][0];
          $Tm[2][1] =  $Tm[2][1] - $TL * $Tm[1][1];
        }
        if (preg_match("#([\-0-9\.]+)\s*Tz#sm", $part, $_Tz)) {
          $Tz = $_Tz[1]/100;
        }

        if ($flag) {
          do {
            $loop = 0;
            if (($text[0] == "(" && $text[1] == " " && $text[2] == ")") || ($text[0] == "(" && $text[1] == ")")) {
              $text[0] = "a";
              $pos1 = strpos($text , ")");
              $pos2 = strpos($text , "(");
              if ($pos2 !== FALSE) {
                $num = substr($text , $pos1 + 1 , $pos2 - $pos1 - 1);
                $text = substr($text , $pos2);
                $Tm[2][0] += -1 * $Tf * $Tz * $num / 1000;
                $loop = 1;
              }
              else {
                $text = "";
              }
            }
          } while ($loop);
        }
        $t[0] = $Tm[0][0] * $CG[0][0] + $Tm[0][1] * $CG[1][0];
        $t[1] = $Tm[0][0] * $CG[0][1] + $Tm[0][1] * $CG[1][1];
        $t[2] = $Tm[1][0] * $CG[0][0] + $Tm[1][1] * $CG[1][0];
        $t[3] = $Tm[1][0] * $CG[0][1] + $Tm[1][1] * $CG[1][1];
        $t[4] = $Tm[2][0] * $CG[0][0] + $Tm[2][1] * $CG[1][0] + $CG[2][0];
        $t[5] = $Tm[2][0] * $CG[0][1] + $Tm[2][1] * $CG[1][1] + $CG[2][1];

        $Trm[0][0] = $t[0] * $Tf * $Tz;
        $Trm[0][1] = $t[1] * $Tf * $Tz;
        $Trm[1][0] = $t[2] * $Tf;
        $Trm[1][1] = $t[3] * $Tf;
        $Trm[2][0] = $t[4];
        $Trm[2][1] = $t[5];

        if ($rotation == 90) {
          $Trm[2][1] = $mediaBox_y - $Trm[2][1];
        }
        if ($Trm[0][0] == 1) {
          $Trm[0][0] = 12;
        }
        if ($Trm[0][0] > $_maxHeight) {
          $Trm[0][0] = $_maxHeight;
        }

        $done = 0;
        for ($height = 0; $height < $Trm[0][0] / 2 && !$done; $height++) {
          if ($texts["y" . ((int)$Trm[2][1] + $height)]) {
            $texts["y" . ((int)$Trm[2][1] + $height)] .= "{" . $Trm[2][0] . "}" . "[" . $Trm[0][0] . "]" . $text;
            $done = 1;
          }
        }
        if (!$done) {
          $texts["y" . (int)$Trm[2][1]] .= "{" . $Trm[2][0] . "}" . "[" . $Trm[0][0] . "]" . "*" . $text;
        }
        if ($text != "( )" && $text != "()") {
          $_textAccordingX["x". $Trm[2][0]]++;
        }
      }
    } while ($type);
  }
  return $texts;
}

/**
 * Implementation of Ascii Hex Decoder.
 * @param $input
 */
function _pdf2text_asciihex_decoder($input) {
  $output = "";
  $isOdd = TRUE;
  $isComment = FALSE;

  for ($i = 0, $codeHigh = -1; $i < strlen($input) && $input[$i] != '>'; $i++) {
    $c = $input[$i];
    if ($isComment) {
      if ($c == '\r' || $c == '\n') {
        $isComment = FALSE;
      }
      continue;
    }

    switch ($c) {
      case '\0': case '\t': case '\r': case '\f': case '\n': case ' ':
      break;
      case '%':
        $isComment = TRUE;
      break;
      default:
        $code = hexdec($c);
        if ($code === 0 && $c != '0') {
          return "";
        }
        if ($isOdd) {
          $codeHigh = $code;
        }
        else {
          $output .= chr($codeHigh * 16 + $code);
        }
        $isOdd = !$isOdd;
      break;
    }
  }

  if ($input[$i] != '>') {
    return "";
  }
  if ($isOdd) {
    $output .= chr($codeHigh * 16);
  }
  return $output;
}

/**
 * Implementation of Ascii 85 Decoder.
 * @param $input
 */
function _pdf2text_ascii85_decoder($input) {
  $output = "";
  $isComment = FALSE;
  $ords = array();

  for ($i = 0, $state = 0; $i < strlen($input) && $input[$i] != '~'; $i++) {
    $c = $input[$i];

    if ($isComment) {
      if ($c == '\r' || $c == '\n') {
        $isComment = FALSE;
      }
      continue;
    }

    if ($c == '\0' || $c == '\t' || $c == '\r' || $c == '\f' || $c == '\n' || $c == ' ') {
      continue;
    }
    if ($c == '%') {
      $isComment = TRUE;
      continue;
    }
    if ($c == 'z' && $state === 0) {
      $output .= str_repeat(chr(0), 4);
      continue;
    }
    if ($c < '!' || $c > 'u') {
      return "";
    }

    $code = ord($input[$i]) & 0xff;
    $ords[$state++] = $code - ord('!');

    if ($state == 5) {
      $state = 0;
      for ($sum = 0, $j = 0; $j < 5; $j++) {
        $sum = $sum * 85 + $ords[$j];
      }
      for ($j = 3; $j >= 0; $j--) {
        $output .= chr($sum >> ($j * 8));
      }
    }
  }
  if ($state === 1) {
    return "";
  }
  elseif ($state > 1) {
    for ($i = 0, $sum = 0; $i < $state; $i++) {
      $sum += ($ords[$i] + ($i == $state - 1)) * pow(85, 4 - $i);
    }
    for ($i = 0; $i < $state - 1; $i++) {
      $ouput .= chr($sum >> ((3 - $i) * 8));
    }
  }
  return $output;
}

/**
 * Implementation of Flate Decoder
 * @param $input
 */
function _pdf2text_flate_decoder($input) {
  return @gzuncompress($input);
}

/**
 * Determine the options of the object
 * @param $object
 */
function _pdf2text_get_object_options($object) {
  $options = array();
  if (preg_match("#<<(.*)>>#ismU", $object, $options)) {
    $options = explode("/", $options[1]);

    @array_shift($options);

    $o = array();
    for ($j = 0; $j < @count($options); $j++) {
      $options[$j] = preg_replace("#\s+#", " ", trim($options[$j]));
      if (strpos($options[$j], " ") !== FALSE) {
        $parts = explode(" ", $options[$j]);
        $o[$parts[0]] = $parts[1];
      }
      else {
        $o[$options[$j]] = TRUE;
      }
    }
    $options = $o;
    unset($o);
  }
  return $options;
}

/**
 * Determine the stream of content to be decoded.
 * @param unknown_type $stream
 * @param unknown_type $object
 */
function _pdf2text_get_decoded_stream($stream, $object) {
  $data = "";
  $flag = 0;
  if (preg_match("#Filter(.*)>>#ismU", $object, $_filter)) {
    $filter = $_filter[1];
    $flag = 1;
  }

  $_stream = $stream;
  if (!$flag) {
    $data = $_stream;
  }
  else {
    if (preg_match("#\[/([a-zA-Z]+)\]#", $filter, $_filter)) {
      $_filter = @ $_filter[1];
    }
    if (preg_match("#/([a-zA-Z]+)#", $filter, $_filter)) {
      $_filter = @ $_filter[1];
    }

    if ($_filter == "ASCIIHexDecode") {
      $_stream = _pdf2text_asciihex_decoder($_stream);
    }
    elseif ($_filter == "ASCII85Decode") {
      $_stream = _pdf2text_ascii85_decoder($_stream);
    }
    elseif ($_filter == "FlateDecode") {
      $_stream = _pdf2text_flate_decoder($_stream);
    }

    $data = $_stream;
  }
  return $data;
}

/**
 * Getting the transformations of decoded data set.
 * @param $transformations
 * @param $stream
 */
function _pdf2text_get_char_transformations(&$transformations, $stream) {
  preg_match_all("#([0-9]+)\s+beginbfchar(.*)endbfchar#ismU", $stream, $chars, PREG_SET_ORDER);
  preg_match_all("#([0-9]+)\s+beginbfrange(.*)endbfrange#ismU", $stream, $ranges, PREG_SET_ORDER);

  for ($j = 0; $j < count($chars); $j++) {
    $count = $chars[$j][1];
    $current = explode("\n", trim($chars[$j][2]));
    for ($k = 0; $k < $count && $k < count($current); $k++) {
      if (preg_match("#<([0-9a-f]{2,4})>\s+<([0-9a-f]{4,512})>#is", trim($current[$k]), $map)) {
        $transformations[str_pad($map[1], 4, "0")] = $map[2];
      }
    }
  }
  for ($j = 0; $j < count($ranges); $j++) {
    $count = $ranges[$j][1];
    $current = explode("\n", trim($ranges[$j][2]));
    for ($k = 0; $k < $count && $k < count($current); $k++) {
      if (preg_match("#<([0-9a-f]{4})>\s+<([0-9a-f]{4})>\s+<([0-9a-f]{4})>#is", trim($current[$k]), $map)) {
        $from = hexdec($map[1]);
        $to = hexdec($map[2]);
        $_from = hexdec($map[3]);

        for ($m = $from, $n = 0; $m <= $to; $m++, $n++) {
          $transformations[sprintf("%04X", $m)] = sprintf("%04X", $_from + $n);
        }
      }
      elseif (preg_match("#<([0-9a-f]{4})>\s+<([0-9a-f]{4})>\s+\[(.*)\]#ismU", trim($current[$k]), $map)) {
        $from = hexdec($map[1]);
        $to = hexdec($map[2]);
        $parts = preg_split("#\s+#", trim($map[3]));

        for ($m = $from, $n = 0; $m <= $to && $n < count($parts); $m++, $n++) {
          $transformations[sprintf("%04X", $m)] = sprintf("%04X", hexdec($parts[$n]));
        }
      }
    }
  }
}

/**
 * Function for handling the given file.
 * @param $filename
 */
function _pdf2text_pdf_to_text($filename) {
  global $_infile;
  global $_document;
  global $_maxHeight;

  $options = array();
  $_maxHeight = 1;
  $_document = "<div style='left: 0px; position:relative;'>";

  // Open the file
  $_infile = @file_get_contents($filename);
  if (empty($_infile)) {
    return "";
  }

  _pdf2text_initialize();
  if (preg_match_all("#trailer(.*)%%EOF#ismU", $_infile, $trailer)) {
    $trailer = @ $trailer[1];
  }

  for ($i = count($trailer) - 1; $i >= 0; --$i) {
    if (stristr($trailer[$i], "Root")) {
      $_trailer = $trailer[$i];
      break;
    }
  }

  if ($_trailer) {
    preg_match("#Root ([0-9]+) 0 R#ismU", $_trailer, $root);
    $root = @$root[1];
  }

  else {
    preg_match("#(.*)/Catalog#ismU", $_infile, $_root);
    $_root = @$_root[1];
    preg_match_all("#([0-9]+) 0 obj#smU", $_root, $roots);
    $roots = $roots[1];
    $root = $roots[count($roots) - 1];
  }
  $object = _pdf2text_search_object($root);

  $catalog = strpos($object, "/Catalog");
  $pages = strstr($object, "/Pages");

  if ($catalog && $pages) {
    preg_match("#([0-9]+) 0 R#ismU", $pages, $page);
    $page = @$page[1];
    $object = _pdf2text_search_object($page);

    if ($object == "" && $page) {
      $object = "Kids ";
      if (preg_match_all("#(.*)/Parent ". $page ." 0 R#smU", $_infile, $pageObjects)) {
        $pageObjects = $pageObjects[1];
        foreach ($pageObjects as $longObj) {
          preg_match_all("#([0-9]+) 0 obj#smU", $longObj, $allObj);
          $allObj = $allObj[1];
          $objNo = $allObj[count($allObj) - 1];
          $object .= $objNo ." 0 R ";
        }
        $object .= ">>";
      }
    }
    _pdf2text_serve_tree_page($object);
  }
  //$_document .= "</div>";
  return $_document;
}

/**
 * Getting the object to be decoded and rendering the content for display.
 * @param $object
 */
function _pdf2text_serve_tree_page($object) {
  global $_document;
  global $_infile;

  $rotation = 0;
  $flag = 0;
  $fonts = array();
  $xobjects = array();
  if (preg_match("#Contents(.*)(/|>>)#smU", $object, $content)) {
    $content = $content[1];
  }
  if (preg_match("#Kids(.*)(/|>>)#ismU", $object, $kids)) {
    $kids = $kids[1];
  }
  if (preg_match("#MediaBox \[ ([0-9]+) ([0-9]+) ([0-9]+) ([0-9]+) \]#smU", $object, $mediaBox)) {
    $mediaBox_x = $mediaBox[3];
    $mediaBox_y = $mediaBox[4];
  }
  if (preg_match("#Resources\s*([0-9]+)\s*0\s*R#smU", $object, $resource)) {
    $resource = $resource[1];
    $resourceLine = _pdf2text_search_object($resource);
    $flag = 1;
  }
  elseif (preg_match("#Resources\s*<<(.*)>>#smU", $object, $resourceLine)) {
    $resourceLine = $resourceLine[1] . ">>";
    $flag = 1;
  }

  if ($flag && preg_match("#Font\s*([0-9]+)\s*0\s*R#smU", $resourceLine, $font)) {
    $font = $font[1];
    $fontObject = _pdf2text_search_object($font);
    if (preg_match("#(<<.*>>)#smU", $fontObject, $fontLine)) {
      $fontLine = $fontLine[1];
      $fonts = _pdf2text_get_object_options($fontLine);
    }
  }
  elseif ($flag && preg_match("#Font\s*(<<.*>>)#smU", $resourceLine, $fontLine)) {
    $fontLine = $fontLine[1];
    $fonts = _pdf2text_get_object_options($fontLine);
  }
  if ($flag && preg_match("#XObject\s*0\s*R#smU", $resourceLine, $xobject)) {
    $xobject = $xobject[1];
    $extObject = _pdf2text_search_object($xobject);
    if (preg_match("#(<<.*>>)#smU", $extObject, $xobjectLine)) {
      $xobjectLine = $xobjectLine[1];
      $xobjects = _pdf2text_get_object_options($xobjectLine);
    }
  }
  elseif ($flag && preg_match("#XObject\s*(<<.*>>)#smU", $resourceLine, $xobjectLine)) {
    $xobjectLine = $xobjectLine[1];
    $xobjects = _pdf2text_get_object_options($xobjectLine);
  }
  if (preg_match("#Rotate\s+([0-9]+)#sm", $object, $rotate)) {
    $rotation = $rotate[1];
  }

  if ($content) {
    if (preg_match_all("#([0-9]+) 0 R#", $content, $pages)) {
      $pages = $pages[1];
      $texts = array();
      $data = "";
      foreach ($pages as $page) {
        $o = _pdf2text_search_object($page);
        $decoded = _pdf2text_get_stream($o);
        $data .= $decoded;
      }
      $texts = _pdf2text_add_content($data, $mediaBox_x, $mediaBox_y, $fonts, $xobjects, $rotation);
      _pdf2text_render_content($texts);
      $_document .= "<font size = 3>&#160&#160&#160&#160&#160&#160&#160_________________________________________________________________________________________________</font><br><br>";
    }
  }

  if ($kids) {
    if (preg_match_all("#([0-9]+) 0 R#ismU", $kids, $pages)) {
      $pages = $pages[1];
      foreach ($pages as $page) {
        $o = _pdf2text_search_object($page);
        _pdf2text_serve_tree_page($o);
      }
    }
  }
}

/**
 * Function for getting the decoded data stream of given object.
 * @param $currentObject
 */
function _pdf2text_get_stream($currentObject) {
  $stream = "";
  if (preg_match("#stream(.*)endstream#ismU", $currentObject, $stream)) {
    $stream = ltrim($stream[1]);
  }
  else {
    $pos1 = strpos($currentObject, "stream");
    $pos2 = strpos($currentObject, "endstream");
    if ($pos1 !== FALSE && $pos2 !== FALSE) {
      $stream = ltrim(substr($currentObject, $pos1 + 6, $pos2 - $pos1 - 6));
    }
  }
  return _pdf2text_get_decoded_stream($stream, $currentObject);
}

/**
 * Add the content of given page in data.
 * @param $data
 * @param $mediaBox_x
 * @param $mediaBox_y
 * @param $fonts
 * @param $xobjects
 * @param $rotation
 */
function _pdf2text_add_content($data, $mediaBox_x, $mediaBox_y, $fonts, $xobjects, $rotation) {
  $l = 0;
  $index = 0;
  $textContainers = array();
  $graphicsContainers = array();
  $texts = array();
  if (!$mediaBox_x || !$mediaBox_y) {
    $mediaBox_x = 612;
    $mediaBox_y = 792;
  }
  if (strlen($data)) {
    if (preg_match_all("#\bBT(.*)\bET#smU", $data, $text)) {
      $textContainers = $text[1];
      preg_match("#(.*)\bBT#smU", $data, $graphics);
      $graphicsContainers[0] = $graphics[1];
      preg_match_all("#\bET(.*)\bBT#smU", $data, $graphics);
      $graphicsContainers = array_merge($graphicsContainers, $graphics[1]);
    }
    else {
      $graphicsContainers[0] = $data;
      $textContainers[0] = '';
    }

    $texts = _pdf2text_get_texts($textContainers, $graphicsContainers, $mediaBox_x, $mediaBox_y, $fonts, $xobjects, $rotation);
    return $texts;
  }
}

/**
 * Searching the object in file.
 * @param $obj_no
 */
function _pdf2text_search_object($obj_no) {
  global $_infile;
  if (preg_match_all("#\b". $obj_no ." 0 obj(.*)endobj#ismU", $_infile, $objects)) {
    $objects = @$objects[1];
    $object = $objects[count($objects)-1];
    return $object;
  }
  else {
    if ($pos1 = strrpos($_infile, $obj_no ." 0 obj")) {
      $object = substr($_infile, $pos1);
      if ($pos2 = strpos($object, "endobj")) {
        $object = substr($object, 0, $pos2);
        return $object;
      }
    }
  }
  return "";
}

/**
 * Adding the content for output.
 * @param $textLine
 * @param $div
 */
function _pdf2text_show_content($textLine, $div) {
  global $_textAccordingX;
  global $_document;

  $doc = "";
  $i = 0;
  $maxSize = 0;
  foreach ($textLine as $key => $t) {
    $isPlain = FALSE;
    $isPoint = FALSE;
    $flag = 0;
    preg_match("#\](.*)#sm" , $t , $text);
    $text = $text[1];
    if (!$i) {
      $xPos = $key;
    }
    elseif ($_textAccordingX["x". $key] > 3 && $text != "( )" && $text != "()") {
    //  $_document .= "<div style='left:". (1.3 * $xPos) ."px;position: absolute; z-index: 1; visibility: show;'>". $doc ."</div>";
      $_document .= $doc;
      $xPos = $key;
      $doc = "";
    }
    elseif ($key - $prevPos - $nc > $div) {
     // $_document .= "<div style='left:". (1.3 * $xPos) ."px;position: absolute; z-index: 1; visibility: show;'>". $doc ."</div>";
     $_document .= $doc;
      $xPos = $key;
      $doc = "";
    }
    $prevPos = $key;
    $i++;
    for ($j = 0, $nc = 0; $j < strlen($t); $j++) {
      $c = $t[$j];
      if (!$isPlain && $c == "-") {
        $flag = 1;
      }
      elseif ($c >= "0" && $c <= "9" && !$isPlain && !$isPoint && $flag) {
        for ($k = 0 ; $j + $k < strlen($t) && $t[$j + $k] >= "0" && $t[$j + $k] <= "9"; ++$k)
          ;
        if ($k>=3) {
          $doc .= " ";
          $nc += $actSize * 0.75;
        }
      }
      else {
        switch ($c) {
          case "(":
            $plain = "";
            $isPlain = TRUE;
            $flag = 0;
            if ($isPoint) {
              $isPoint = FALSE;
            }
          break;
          case ")":
            $doc .= $plain;
            $isPlain = FALSE;
          break;
          case "\\":
            $c2 = $t[$j + 1];
            if (in_array($c2, array("\\", "(", ")"))) {
              $plain .= $c2;
            }
            elseif ($c2 >= '0' && $c2 <= '9') {
              $oct = preg_replace("#[^0-9]#", "", substr($t, $j + 1, 3));
              $j += strlen($oct) - 1;
              if ($oct > "027") {
                $plain .= html_entity_decode("&#" . octdec($oct) . ";");
              }
            }
            $nc += $actSize * 0.75;
            $j++;
          break;
          case ".":
            if ($isPlain) {
              $plain .= $c;
              $nc += $actSize / 2;
            }
            else {
              $isPoint = TRUE;
            }
          break;
          case "[":
            if ($isPlain) {
              $plain .= $c;
              $nc += $actSize / 2;
            }
            else {
              for ($k = 1, $actSize = NULL; $j + $k < strlen($t) && $t[$j + $k] != "]"; $k++) {
                $actSize .= $t[$j + $k];
              }
              $size = round($actSize / $div);
              if ($size < 2) {
                $size = 2;
              }
              elseif ($size > 5) {
                $size = 5;
              }
              if ($size > $maxSize) {
                $maxSize = $size;
              }
              //$doc .= "<font size = " . $size . ">";
              $j += $k;
            }
          break;
          case "<":
            if ($isPlain) {
              $plain .= "&#60";
              $nc += $actSize / 2;
            }
          break;
          case ">":
            if ($isPlain) {
              $plain .= "&#62";
              $nc += $actSize / 2;
            }
          break;
          default:
            if ($isPlain) {
              $plain .= $c;
              $nc += $actSize * 0.75;
            }
          break;
        }
      }
    }
  }
  //$_document .= "<div style='left:". (1.3 * $xPos) ."px;position: absolute; z-index: 1; visibility: show;'>". $doc ."</div>";
  $_document .= $doc;
  return $maxSize;
}

/**
 * Rendering the content to output.
 * @param $texts
 */
function _pdf2text_render_content($texts) {
  global $_document;
  $temp = array();

  if ($texts != NULL) {
    foreach ($texts as $key => $t) {
      $index = substr($key, 1);
      $temp[$index] = $t;
    }
    krsort($temp);

    foreach ($temp as $key => $t) {
      $textLine = array();
      $isPlain = FALSE;
      $isSize = FALSE;
      for ($i = 0; $i < strlen($t); $i++) {
        if ($t[$i] == "(") {
          $isPlain = TRUE;
          $plain .= $t[$i];
        }
        elseif ($t[$i] == ")") {
          $isPlain = FALSE;
          $plain .= $t[$i];
          $textLine[$xPos] .= $plain;
          $plain = NULL;
        }
        elseif ($t[$i] == "{") {
          if (!$isPlain) {
            for ($k = 1, $xPos = ""; $i + $k < strlen($t) && $t[$i + $k] != "}"; $k++) {
              $xPos .= $t[$i + $k];
            }
            $i += $k;
          }
          else {
            $plain .= $t[$i];
          }
        }
        else {
          $plain .= $t[$i];
        }
      }
      ksort($textLine);
      $div = 4;
      _pdf2text_show_content($textLine, $div);
      $_document .= "<br>";
      //$_document .= "<font size =". $maxSize ."><br></font>";
    }
  }
}

/**
 * Converting the text to unicode.
 * @param unknown_type $text
 * @param unknown_type $o
 * @param unknown_type $flag
 */
function _pdf2text_convert_to_unicode(&$text, $o, $flag) {
  $txt = "";
  $diff = array();

  $encoding = _pdf2text_find_encoding($o, $diff, $type);
  if (!$flag) {
    $txt = substr($text, 1, strlen($text) - 2);
    $text = "(" . _pdf2text_convert_part($txt, $encoding, $diff, $type) . ")";
  }
  else {
    for ($i = 0, $j = 0, $k = 0; $i < strlen($text); ++$i) {
      if ($text[$i] == "(" && ($i == 0 || $text[$i - 1] != "\\")) {
        $leftBrace[$j++] = $i;
      }
      elseif ($text[$i] == ")" && $text[$i - 1] != "\\") {
        $rightBrace[$k++] = $i;
      }
    }
    if ($j != $k) {
      return;
    }
    for ($j = 0, $Tm = 0; $j < $k; ++$j) {
      if ($leftBrace[$j] > $rightBrace[$j]) {
        return;
      }
      $txtPart = substr($text, $leftBrace[$j] + 1, $rightBrace[$j] - $leftBrace[$j] - 1);
      if ($j != $k - 1) {
        $digPart = substr($text, $rightBrace[$j] + 1, $leftBrace[$j + 1] - $rightBrace[$j] - 1);
      }
      else {
        $digPart = "";
      }
      $txt .= "(" . _pdf2text_convert_part($txtPart, $encoding, $diff, $type) . ")" . $digPart;
    }
    $text = $txt;
  }
}

/**
 * Convert partial input to output text.
 * @param $part
 * @param $encoding
 * @param $diff
 * @param $type
 */
function _pdf2text_convert_part($part, $encoding, $diff, $type) {
  global $_uniCode;
  global $_winToPdf;
  global $_pdfDocToUni;
  $txt = "";

  for ($i = 0; $i < strlen($part); ++$i) {
    $t = 0;
    $c = $part[$i];
    if ($part[$i] == "\\") {
      $c2 = $part[$i + 1];
      if ($c2 == "n") {
        $u = "012";
        ++$i;
        $t = 1;
      }
      elseif ($c2 == "r") {
        $u = "015";
        ++$i;
        $t = 1;
      }
      elseif ($c2 == "t") {
        $u = "011";
        ++$i;
        $t = 1;
      }
      elseif ($c2 == "b") {
        $u = "010";
        ++$i;
        $t = 1;
      }
      elseif ($c2 == "f") {
        $u = "014";
        ++$i;
        $t = 1;
      }
      elseif ($c2 >= '0' && $c2 <= '9') {
        $u = preg_replace("#[^0-9]#", "", substr($part, $i + 1, 3));
        if ($u > "027") {
          $txt .= html_entity_decode("&#" . octdec($u) . ";");
        }
        $i += strlen($u);
        $t = 0;
      }
      elseif ($c2 == "(") {
        $u = "050";
        ++$i;
        $t = 1;
      }
      elseif ($c2 == ")") {
        $u = "051";
        ++$i;
        $t = 1;
      }
      else {
        $u = "134";
        $t = 1;
      }
    }
    else {
      $u = $_uniCode[$c];
      $t = 1;
    }
    if ($t) {
      if ($type && isset($diff[octdec($u)])) {
        if ($diff[octdec($u)] == "(" || $diff[octdec($u)] == ")") {
          $txt .= "\\";
        }
        $txt .= $diff[octdec($u)];
      }
      elseif ($encoding == "WinAnsiEncoding") {
        if (isset($_winToPdf[$u])) {
          $u = $_winToPdf[$u];
        }
        if (isset($_pdfDocToUni[$oct])) {
          $u = $_pdfDocToUni[$u];
        }
        if ($u > "027") {
          if (html_entity_decode("&#" . octdec($u) . ";") == "(" || html_entity_decode("&#" . octdec($u) . ";") == ")") {
            $txt .= "\\";
          }
          $txt .= html_entity_decode("&#" . octdec($u) . ";");
        }
      }
      elseif ($encodinf == "PDFDocEncoding") {
        if (isset($_pdfDocToUni[$u])) {
          $oct = $_pdfDocToUni[$u];
        }
        if ($u > "027") {
          if (html_entity_decode("&#" . octdec($u) . ";") == "(" || html_entity_decode("&#" . octdec($u) . ";") == ")") {
            $txt .= "\\";
          }
          $txt .= html_entity_decode("&#" . octdec($oct) . ";");
        }
      }
      elseif ($u > "0027" && !$type) {
        if (html_entity_decode("&#" . octdec($u) . ";") == "(" || html_entity_decode("&#" . octdec($u) . ";") == ")") {
          $txt .= "\\";
        }
        $txt .= html_entity_decode("&#" . octdec($u) . ";");
      }
    }
  }
  return $txt;
}

/**
 * Finding the encoding of given object.
 * @param $o
 * @param $diff
 * @param $type
 */
function _pdf2text_find_encoding($o, &$diff, &$type) {
  global $_trans;
  $type = 0;

  if (preg_match("#Encoding\s*([0-9]+)\s*0\s*R#smU", $o, $obj)) {
    $obj = $obj[1];
    $o = _pdf2text_search_object($obj);
  }
  if (strpos($o, "WinAnsiEncoding") !== FALSE) {
    $encoding = "WinAnsiEncoding";
  }
  elseif (strpos($o, "PDFDocEncoding") !== FALSE) {
    $encoding = "PDFDocEncoding";
  }
  else {
    $encoding = "";
  }
  if ($encoding != "") {
    if (preg_match("#Differences\s*\[(.*)\]#smU", $o, $differences)) {
      $differences = $differences[1];
      $type = 1;
      $char = explode("/", $differences);
      foreach ($char as $c) {
        $flag = 0;
        if (is_numeric(trim($c))) {
          $counter = trim($c);
        }
        else {
          if (preg_match("#([a-zA-Z0-9]+)\s+([0-9]+)#sm", $c, $parts)) {
            $c = $parts[1];
            $flag = 1;
          }
          if (isset($_trans[trim($c)])) {
            $diff[$counter++] = $_trans[trim($c)];
          }
          elseif (strlen(trim($c)) == 1) {
            $diff[$counter++] = trim($c);
          }
          else {
            $diff[$counter++] = "";
          }
          if ($flag) {
            $counter  = $parts[2];
          }
        }
      }
    }
    return trim($encoding);
  }
}
