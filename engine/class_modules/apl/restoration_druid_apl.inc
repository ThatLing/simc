action_priority_list_t* precombat = get_action_priority_list( "precombat" );
action_priority_list_t* def = get_action_priority_list( "default" );
action_priority_list_t* cat = get_action_priority_list( "cat" );
action_priority_list_t* owl = get_action_priority_list( "owl" );

precombat->add_action( "cat_form,if=talent.feral_affinity.enabled" );
precombat->add_action( "prowl,if=talent.feral_affinity.enabled" );
precombat->add_action( "moonkin_form,if=talent.balance_affinity.enabled" );

def->add_action( "berserking" );
def->add_action( "use_items,if=!buff.prowl.up&!buff.shadowmeld.up" );
def->add_action( "potion" );
def->add_action( "run_action_list,name=owl,if=talent.balance_affinity.enabled" );
def->add_action( "run_action_list,name=cat,if=talent.feral_affinity.enabled" );
def->add_action( "convoke_the_spirits,if=(buff.heart_of_the_wild.up|cooldown.heart_of_the_wild.remains>60-30*runeforge.celestial_spirits|!talent.heart_of_the_wild.enabled)&(!equipped.soulleting_ruby|cooldown.soulleting_ruby.remains<114&!cooldown.soulleting_ruby.ready)" );
def->add_action( "adaptive_swarm,target_if=!dot.adaptive_swarm_damage.ticking&!action.adaptive_swarm_damage.in_flight&(!dot.adaptive_swarm_heal.ticking|dot.adaptive_swarm_heal.remains>5|dot.adaptive_swarm_heal.stack=1)|dot.adaptive_swarm_damage.stack<3&dot.adaptive_swarm_damage.remains<3&dot.adaptive_swarm_damage.ticking" );
def->add_action( "ravenous_frenzy,if=(buff.heart_of_the_wild.up|cooldown.heart_of_the_wild.remains>60|!talent.heart_of_the_wild.enabled)" );
def->add_action( "kindred_spirits,if=(buff.heart_of_the_wild.up|cooldown.heart_of_the_wild.remains>60|!talent.heart_of_the_wild.enabled)" );
def->add_action( "sunfire,target_if=refreshable" );
def->add_action( "moonfire,target_if=refreshable" );
def->add_action( "wrath" );
def->add_action( "moonfire" );

cat->add_action( "rake,if=buff.shadowmeld.up|buff.prowl.up" );
cat->add_action( "auto_attack,if=!buff.prowl.up&!buff.shadowmeld.up" );
cat->add_action( "adaptive_swarm,target_if=!dot.adaptive_swarm_damage.ticking&!action.adaptive_swarm_damage.in_flight&(!dot.adaptive_swarm_heal.ticking|dot.adaptive_swarm_heal.remains>5|dot.adaptive_swarm_heal.stack=1)|dot.adaptive_swarm_damage.stack<3&dot.adaptive_swarm_damage.remains<3&dot.adaptive_swarm_damage.ticking" );
cat->add_action( "kindred_spirits,if=(buff.heart_of_the_wild.up|cooldown.heart_of_the_wild.remains>60|!talent.heart_of_the_wild.enabled)" );
cat->add_action( "sunfire,target_if=refreshable&target.time_to_die>5" );
cat->add_action( "moonfire,target_if=refreshable&time_to_die>12&(spell_targets.swipe_cat<=4|energy<50)&!ticking|(prev_gcd.1.sunfire&remains<duration*0.8&spell_targets.sunfire=1)" );
cat->add_action( "sunfire,if=prev_gcd.1.moonfire&remains<duration*0.8" );
cat->add_action( "ravenous_frenzy,if=(buff.heart_of_the_wild.up|cooldown.heart_of_the_wild.remains>60|!talent.heart_of_the_wild.enabled)" );
cat->add_action( "convoke_the_spirits,if=(buff.heart_of_the_wild.up|cooldown.heart_of_the_wild.remains>60-30*runeforge.celestial_spirits|!talent.heart_of_the_wild.enabled)&buff.cat_form.up&energy<50&(combo_points<5&dot.rip.remains>5|spell_targets.swipe_cat>1)" );
cat->add_action( "heart_of_the_wild,if=(cooldown.convoke_the_spirits.remains<30|!covenant.night_fae)&!buff.heart_of_the_wild.up&dot.sunfire.ticking&(dot.moonfire.ticking|spell_targets.swipe_cat>4)" );
cat->add_action( "cat_form,if=!buff.cat_form.up&energy>50" );
cat->add_action( "wrath,if=!buff.cat_form.up" );
cat->add_action( "ferocious_bite,if=(combo_points>3&target.1.time_to_die<3|combo_points=5&energy>=50&dot.rip.remains>10)&spell_targets.swipe_cat<4" );
cat->add_action( "rip,target_if=((refreshable|energy>90&remains<=10)&(combo_points=5&time_to_die>remains+24|(remains+combo_points*4<time_to_die&remains+4+combo_points*4>time_to_die))|!ticking&combo_points>2+spell_targets.swipe_cat*2)&spell_targets.swipe_cat<11" );
cat->add_action( "rake,target_if=refreshable&time_to_die>10&(combo_points<5|remains<1)&spell_targets.swipe_cat<5" );
cat->add_action( "swipe_cat,if=spell_targets.swipe_cat>=2" );
cat->add_action( "shred,if=combo_points<5|energy>90" );

owl->add_action( "heart_of_the_wild,if=(cooldown.convoke_the_spirits.remains<30|cooldown.convoke_the_spirits.remains>90|!covenant.night_fae)&!buff.heart_of_the_wild.up" );
owl->add_action( "cat_form,if=runeforge.oath_of_the_elder_druid&!buff.oath_of_the_elder_druid.up" );
owl->add_action( "moonkin_form,if=!buff.moonkin_form.up" );
owl->add_action( "convoke_the_spirits,if=(buff.heart_of_the_wild.up|cooldown.heart_of_the_wild.remains>60-30*runeforge.celestial_spirits|!talent.heart_of_the_wild.enabled)&(buff.eclipse_solar.up|spell_targets.starfire>1&eclipse.in_any|spell_targets.starfire>=9)&(!equipped.soulleting_ruby|cooldown.soulleting_ruby.remains<114&!cooldown.soulleting_ruby.ready)&(buff.heart_of_the_wild.remains>4|!buff.heart_of_the_wild.up)" );
owl->add_action( "adaptive_swarm,target_if=!dot.adaptive_swarm_damage.ticking&!action.adaptive_swarm_damage.in_flight&(!dot.adaptive_swarm_heal.ticking|dot.adaptive_swarm_heal.remains>5|dot.adaptive_swarm_heal.stack=1)|dot.adaptive_swarm_damage.stack<3&dot.adaptive_swarm_damage.remains<3&dot.adaptive_swarm_damage.ticking" );
owl->add_action( "ravenous_frenzy,if=(buff.heart_of_the_wild.up|cooldown.heart_of_the_wild.remains>60|!talent.heart_of_the_wild.enabled)" );
owl->add_action( "kindred_spirits,if=(buff.heart_of_the_wild.up|cooldown.heart_of_the_wild.remains>60|!talent.heart_of_the_wild.enabled)" );
owl->add_action( "starsurge,if=spell_targets.starfire<6|!eclipse.in_lunar&spell_targets.starfire<8" );
owl->add_action( "moonfire,target_if=refreshable&target.time_to_die>5&(spell_targets.starfire<5|!eclipse.in_lunar&spell_targets.starfire<7)" );
owl->add_action( "sunfire,target_if=refreshable&target.time_to_die>5" );
owl->add_action( "wrath,if=eclipse.in_solar&spell_targets.starfire=1|eclipse.lunar_next|eclipse.any_next&spell_targets.starfire>1" );
owl->add_action( "starfire" );
