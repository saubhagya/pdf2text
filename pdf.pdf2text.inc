<?php

require_once "object.pdf2text.inc";
require_once "dictionary.pdf2text.inc";
require_once "pdfArray.pdf2text.inc";

class Pdf {
  private $file;
  private $text;
  private $objects = array();

  public function __construct($filename) {
    $this->file = @file_get_contents($filename);
    $this->text = "";
    $this->setObjects();
  }

  private function setObject($objNo) {
    if ($objStartPos = strrpos($this->file, "\n" . $objNo . " 0 obj")) {
      $tailFile = substr($this->file, $objStartPos);
      if ($objEndPos = strpos($tailFile, "endobj")) {
        $object = new Object(substr($tailFile, 0, $objEndPos));
        return $object;
      }
    }
    return "";
  }

  private function setObjects() {
    $pattern = "#([0-9]+) 0 obj#";
    $subject = $this->file;

    preg_match_all($pattern, $subject, $matches);
    foreach ($matches[1] as $objNo) {
      $this->objects[$objNo] = $this->setObject($objNo);
    }
  }

  private function getRoot() {
    $trailerDictionaries = array();

    if (preg_match_all("#trailer\s<<(.*)>>#ismU", $this->file, $matches)) {
      $trailerDictionaries = $matches[1];
    }
    for ($i = count($trailerDictionaries) - 1; $i >= 0; $i--) {
      $trailerDictionary = new Dictionary("<< " . $trailerDictionaries[$i] . " >>");
      return $trailerDictionary->getProperty("Root");
    }
    return "";
  }

  private function getIndirectObjNo($string) {
    if (preg_match("#([0-9]+) 0 R#", $string, $match)) {
      return $match[1];
    }
    return "";
  }

  private function getContent($textContainer) {
    $text = "";

    if (preg_match_all("#\((.*)\)#smU", $textContainer, $matches)) {
      $contents = $matches[1];
      foreach ($contents as $content) {
        $text .= $content;
      }
    }
    return $text;
  }

  private function addContent($content) {
    if (preg_match_all("#\bBT(.*)\bET#smU", $content, $matches)) {
      $textContainers = $matches[1];
      foreach ($textContainers as $textContainer) {
        $this->text .= $this->getContent($textContainer) . "<br>";
      }
    }
  }

  private function servePageObject($pageNo) {
    $pageObject = $this->objects[$pageNo];
    $pageDictionary = $pageObject->getDictionary();
    $contents = $pageDictionary->getProperty("Contents");
    $kids = $pageDictionary->getProperty("Kids");
    $data = "";

    if (!empty($contents)) {
      if (isArrayType($contents)) {
        $contentArr = new PdfArray($contents);
        $contentArrElements = $contentArr->getElements();
        foreach ($contentArrElements as $contentArrElement) {
          $contentPageNo = $this->getIndirectObjNo($contentArrElement);
          $data .= $this->objects[$contentPageNo]->getDecodedStream();
        }
      } else {
        $contentPageNo = $this->getIndirectObjNo($contents);
        $data .= $this->objects[$contentPageNo]->getDecodedStream();
      }
      $this->addContent($data);
    }

    if (!empty($kids)) {
      if (isArrayType($kids)) {
        $kidsArr = new PdfArray($kids);
        $kidsArrElements = $kidsArr->getElements();
        foreach ($kidsArrElements as $kidsArrElement) {
          $kidPageNo = $this->getIndirectObjNo($kidsArrElement);
          $this->servePageObject($kidPageNo);
        }
      } else {
        $kidPageNo = $this->getIndirectObjNo($kids);
        $this->servePageObject($kidPageNo);
      }
    }
  }

  public function getText() {
    $root = $this->getRoot();
    $rootObjNo = $this->getIndirectObjNo($root);
    $rootObj = $this->objects[$rootObjNo];
    $rootObjDictionary = $rootObj->getDictionary();

    $pages = $rootObjDictionary->getProperty("Pages");
    $type = $rootObjDictionary->getProperty("Type");

    if ($type == "Catalog"  && !empty($pages)){
      $pageNo = $this->getIndirectObjNo($pages);
      $this->servePageObject($pageNo);
    }
    return $this->text;
  }
}
