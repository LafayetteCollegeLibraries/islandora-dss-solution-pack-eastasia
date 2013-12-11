<?php

require 'mods.inc';

/**
 * @file
 * Implements a simple class for working with Dublin Core data.
 *
 * Inspiration and design shamelessly stolen from the Dublin Core class within Islandora
 */

/**
 * LdrMods Class
 */
class LdrMods extends Mods {

  /**
   * Constructor.
   *
   * @param string $mods_xml
   *   The Dublin Core XML.
   */
  public function __construct($mods_xml = NULL) {
    if (!empty($mods_xml)) {
      $this->mods = self::importFromXMLString($mods_xml);
    }
  }

  /**
   * Creates a new instance of the class by parsing mods_xml.
   *
   * @param string $mods_xml
   *   Dublin Core XML.
   *
   * @return DublinCore
   *   The instantiated object.
   */
  public static function importFromXMLString($mods_xml) {

    $mods_doc = new SimpleXMLElement($mods_xml);

    $mods_doc->registerXPathNamespace("xml", "http://www.w3.org/XML/1998/namespace");
    $mods_doc->registerXPathNamespace("mods", "http://www.loc.gov/mods/v3"); //http://www.loc.gov/mods/v3

    if( property_exists($mods_doc, 'mods')) {

      $mods_doc = $mods_doc->mods;
    }

    $mods_array = array();

    $template_map = array(
			  /*
			    LDR Collections
			   */
                          'dc.contributors.author' => array('xpath' => "./mods:name/mods:role/mods:roleTerm[text()='aut']/../../mods:namePart",
							    'facet' => true),

			  'affiliation' => './mods:affiliation',

                          'dc.contributors.other' => array('xpath' => "./mods:name/mods:role/mods:roleTerm[text()='oth']/../../mods:namePart",
							   'facet' => true),

			  'dc.title' => "./mods:titleInfo/mods:title",

			  'dc.date.accessioned' => array('xpath' => "./mods:originInfo/mods:dateCaptured",
							 'facet' => true,
							 'date' => true),
			  'dc.date.available' => array('xpath' => "./mods:originInfo/mods:dateIssued",
							 'facet' => true,
							 'date' => true),

			  'dc.identifier.citation' => './mods:note[@type="citation/reference"]',
			  
			  'dc.identifier.uri' => './mods:identifier[@type="uri"]',
			  'dc.identifier.doi' => './mods:identifier[@type="doi"]',
			  'dc.identifier.issn' => './mods:identifier[@type="issn"]',
			  'dc.identifier.isbn' => './mods:identifier[@type="isbn"]',

			  'dc.publisher' => './mods:publisher',

			  'dc.type' => './mods:genre',

			  'dc.description.abstract' => './mods:abstract',

			  'dc.subject' => './mods:subject/mods:topic',

			  'dc.subject' => './mods:subject/mods:topic',

			  'dc.relation.ispartofseries' => './mods:relatedItem/mods:title',

			  'dc.description' => './mods:note[@type="contents"]',
                          );

    foreach($template_map as $facet => $value) {

      $is_facet = false;
      $is_date = false;

      if(is_array($value)) {

	$xpath = $value['xpath'];
	$is_facet = $value['facet'];

	if(array_key_exists('date', $value)) {

	  $is_date = $value['date'];
	}
      } else {

	$xpath = $value;
      }

      $map = function($element) {

        $lang = 'en-US';

        // Need to scrape
        // Refactor
        $m = preg_match('/xml\:lang\="(.+)"/', $element->asXML(), $matches);

        if($m) {

          $lang = $matches[1];
        }

        return array('label' => $element->getName(),
                     'language' => $lang,
                     'value' => (string) $element,
		     'class' => drupal_strtolower(preg_replace('/[^A-Za-z0-9]/', '-', $element->getName())));
      };

      $mods_elements = array_map($map, $mods_doc->xpath($xpath));

      // Refactor
      foreach($mods_elements as $i => &$element) {

	if($is_facet) {
	  
	  $element['facet'] = $facet;
	}

	if($is_date) {
	  
	  $element['date_value'] = date('F, Y', strtotime($element['value']));
	}

	// Specific to migrated collections
	$element['label'] = $facet;
      }

      $mods_array = array_merge($mods_array, $mods_elements);
    }

    return $mods_array;
  }
  }
