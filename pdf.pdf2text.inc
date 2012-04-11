<?php

require_once "object.pdf2text.inc";
require_once "dictionary.pdf2text.inc";
require_once "pdfArray.pdf2text.inc";
require_once "textContainer.pdf2text.inc";
require_once "fontObject.pdf2text.inc";

class Pdf {
  private $file;
  private $text;

  public function __construct($filename) {
    $this->file = @file_get_contents($filename);
    $this->text = "";
  }

  private function getObject($objNo) {
    if (!empty($objNo) && (($objStartPos = strrpos($this->file, "\n" . $objNo . " 0 obj")) || 
           ($objStartPos = strrpos($this->file, "\r" . $objNo . " 0 obj")))) {
      $tailFile = substr($this->file, $objStartPos);
      if ($objEndPos = strpos($tailFile, "endobj")) {
        $object = new Object(substr($tailFile, 0, $objEndPos));
        return $object;
      }
    }
    return "";
  }

  private function getRoot() {
    $trailerDictionaries = array();

    if (preg_match_all("#trailer\s<<(.*)>>#ismU", $this->file, $matches)) {
      $trailerDictionaries = $matches[1];
    }
    for ($i = count($trailerDictionaries) - 1; $i >= 0; $i--) {
      $trailerDictionary = new Dictionary("<< " . $trailerDictionaries[$i] . " >>");
      $root = $trailerDictionary->getProperty("Root");
      if (!empty($root)) {
        return $root;
      }
    }
    return "";
  }

  private function getContent($textObject, $fontObjNo) {
    $fontObj = new FontObject($this->getObject($fontObjNo));
    $textContainer = new TextContainer($textObject, $fontObj);
    return $textContainer->getText();
  }

  private function addContent($content, $fontObjNo) {
    if (preg_match_all("#\bBT(.*)\bET#smU", $content, $matches)) {
      $textObjects = $matches[1];
      foreach ($textObjects as $textObject) {
        $this->text .= $this->getContent($textObject, $fontObjNo) . "<br />";
      }
    }
  }

  private function servePageObject($pageNo) {
    $pageObject = $this->getObject($pageNo);
    $pageDictionary = $pageObject->getDictionary();
    $contents = $pageDictionary->getProperty("Contents");
    $kids = $pageDictionary->getProperty("Kids");

    if (!empty($contents)) {
      $resources = new Dictionary($pageDictionary->getProperty("Resources"));
      $font = $resources->getProperty("Font");
      $fontObjNo = "";
      if (!empty($font)) {
        $fontObjNo = getIndirectObjNo($font);
      }
      $data = "";
      
      if (isArrayType($contents)) {
        $contentArr = new PdfArray($contents);
        $contentArrElements = $contentArr->getElements();
        foreach ($contentArrElements as $contentArrElement) {
          $contentPageNo = getIndirectObjNo($contentArrElement);
          $data .= $this->getObject($contentPageNo)->getDecodedStream();
        }
      } else {
        $contentPageNo = getIndirectObjNo($contents);
        $data .= $this->getObject($contentPageNo)->getDecodedStream();
      }
      $this->addContent($data, $fontObjNo);
    }

    if (!empty($kids)) {
      if (isArrayType($kids)) {
        $kidsArr = new PdfArray($kids);
        $kidsArrElements = $kidsArr->getElements();
        foreach ($kidsArrElements as $kidsArrElement) {
          $kidPageNo = getIndirectObjNo($kidsArrElement);
          $this->servePageObject($kidPageNo);
        }
      } else {
        $kidPageNo = getIndirectObjNo($kids);
        $this->servePageObject($kidPageNo);
      }
    }
  }

  public function getText() {
    $root = $this->getRoot();
    $rootObjNo = getIndirectObjNo($root);
    $rootObj = $this->getObject($rootObjNo);
    $rootObjDictionary = $rootObj->getDictionary();

    $pages = $rootObjDictionary->getProperty("Pages");
    $type = $rootObjDictionary->getProperty("Type");

    if ($type == "Catalog"  && !empty($pages)){
      $pageNo = getIndirectObjNo($pages);
      $this->servePageObject($pageNo);
    }
    return $this->text;
  }
}
