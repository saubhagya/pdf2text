<?php

require_once "decoder.pdf2text.inc";

class Object {
  private $data;
  private $dictionary;

  public function __construct($data) {
    $this->data = trim($data);
    $dictionaryData = "";

    if (preg_match("#<<(.*)>>$#sm", $this->data, $match)) {
      $dictionaryData = "<< " . $match[1] . " >>";
    }
    $this->dictionary = new Dictionary($dictionaryData);
  }

  private function getStream() {
    $stream = "";

    $stream_start_pos = strpos($this->data, "stream");
    $stream_end_pos = strpos($this->data, "endstream");
    if ($stream_start_pos !== FALSE && $stream_end_pos !== FALSE) {
      $stream = ltrim(substr($this->data, $stream_start_pos + 6, $stream_end_pos - $stream_start_pos - 6));
    }
    return $stream;
  }

  public function getData() {
    return $this->data;
  }

  public function getDictionary() {
    return $this->dictionary;
  }

  public function getDecodedStream() {
    $filter = $this->getDictionary()->getProperty("Filter");
    $stream = $this->getStream();
    $decoder = new Decoder($stream);
    return $decoder->decode($filter);
  }
}
