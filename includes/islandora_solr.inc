/**
 * @file
 * Hook implementations and functions for islandora_solr
 * @author griffinj@lafayette.edu
 *
 */

/**
 * Callback for islandora_solr_search
 * @see islandora_solr()
 * Decouple and refactor
 * 
 */
function _dss_eastasia_islandora_solr($query=NULL) {

  drupal_load('module', 'islandora_solr');

  global $_islandora_solr_queryclass;
  drupal_add_css(drupal_get_path('module', 'islandora_solr') . '/css/islandora_solr.theme.css');

  // Url parameters.
  $params = $_GET;

  // Default sorting
  // Refactor
  if(!array_key_exists('sort', $params)) {

    $params['sort'] = 'dc.title asc';
  }

  // get profiles
  $primary_profiles = module_invoke_all('islandora_solr_primary_display');
  $secondary_profiles = module_invoke_all('islandora_solr_secondary_display');

  // Get the preferred display profile
  // Order: First choice is what's in the ?profile query var
  //        Second choice is the primary display profile
  //        Third choice is the default IslandoraSolrResults
  $enabled_profiles = array();
  // get enabled displays
  $primary_display_array = variable_get('islandora_solr_primary_display_table', array());
  // if it's set, we take these values
  if (isset($primary_display_array['enabled'])) {
    foreach ($primary_display_array['enabled'] as $key => $value) {
      if ($key === $value) {
        $enabled_profiles[] = $key;
      }
    }
  }

  // Set primary display
  // check if display param is an valid, enabled profile. Else show default.
  if (isset($params['display']) && in_array($params['display'], $enabled_profiles)) {
    $islandora_solr_primary_display = $params['display'];
  }
  else {
    $islandora_solr_primary_display = variable_get('islandora_solr_primary_display', 'default');
    // unset invalid parameter
    unset($params['display']);
  }

  // Build and execute Apache Solr query.
  // this populates the GLOBAL!!!!!!!
  $_islandora_solr_queryclass = new IslandoraSolrQueryProcessor();
  $_islandora_solr_queryclass->buildAndExecuteQuery($query, $params);

  if (empty($_islandora_solr_queryclass->islandoraSolrResult)) {
    return t('Error searching solr index.');
  }

  // TODO: Also filter secondary displays against those checked in the configuration options.
  if (isset($_GET['solr_profile']) && isset($secondary_profiles[$_GET['solr_profile']])) {
    $profile = $secondary_profiles[$_GET['solr_profile']];
  }
  elseif (isset($primary_profiles[$islandora_solr_primary_display])) {
    $profile = $primary_profiles[$islandora_solr_primary_display];
  }
  else {
    drupal_set_message(check_plain(t('There is an error in the solr search configuration: the display profile is not found.')), 'error');
    $profile = $primary_profiles['default'];
  }
  // Include the file for the display profile
  require_once(drupal_get_path('module', $profile['module']) . '/' . $profile['file']);

  // Get display class and function from current display
  $solr_class = $profile['class'];
  $solr_function = $profile['function'];


  // check if the display's class exists
  $use_default_display = TRUE;
  if (class_exists($solr_class)) {
    $implementation = new $solr_class();
    // check if the display's method exists
    if (method_exists($implementation, $solr_function)) {
      // implement results
      $output = $implementation->$solr_function($_islandora_solr_queryclass);
      $use_default_display = FALSE;
    }
  }

  // Class and method are not found: use default
  if ($use_default_display) {
    $results_class = new IslandoraSolrResults();
    $output = $results_class->displayResults($_islandora_solr_queryclass);
  }

  // debug dump
  if (variable_get('islandora_solr_debug_mode', 0)) {
    $message = t('Params: <br /><pre>!debug</pre>', array('!debug' => print_r($_islandora_solr_queryclass->solrParams, TRUE)));
    drupal_set_message(filter_xss($message, array('pre', 'br')), 'status');
  }

  return $output;
}
