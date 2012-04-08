<?php

require_once "pdf.pdf2text.inc";

// Give PDF filename with path.
$filename = "";
$pdf = new Pdf(urldecode($filename));
print $pdf->getText();
