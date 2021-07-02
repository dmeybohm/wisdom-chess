<template>
  <div v-if="selected_search !== null">
    <div class="columns">
      <div class="column">
        <PositionList :fen="selected_search.fen" :analyze_depth="-1" :positions="positions" @selected="select_position"/>
      </div>
      <div class="column" v-for="(deeper_search, depth) in deeper_searches" :key="depth">
        <PositionList :fen="selected_search.fen" :analyze_depth="depth"
                      :positions="deeper_search" @selected="select_position" />
      </div>
    </div>

  </div>
</template>

<script>
import PositionList from "./PositionList";

export default {
  name: "MoveStack",
  components: {PositionList},
  data: function() {
    return {
      // todo make multidimensional
      positions: [],
      deeper_searches: []
    }
  },
  props: {
    url: String,
    selected_search: Object
  },
  watch: {
    selected_search: function(new_val, old_val) {
      console.log(new_val, old_val);
      let decisionId = encodeURIComponent(this.selected_search.decision_id);
      let url = this.url + '/api/fetch.php?object=positions&decision_id='+ decisionId;
      fetch(url)
          .then(data => data.json())
          .then(data => {
            data.forEach(position => position.move_list = [position.move]);
            this.positions = data;
            this.deeper_searches = [];
          })
    }
  },
  methods: {
    // select_position: function(position, analyze_depth) {
    select_position: function(position) {
      let parentPositionId = encodeURIComponent(position.id);
      let url = this.url + '/api/fetch.php?object=positions&position_id='+ parentPositionId;
      fetch(url)
          .then(data => data.json())
          .then(function(data) {
            data.forEach(function(new_position) {
              new_position.move_list = position.move_list.concat(new_position.move);
            });
            // analyze_depth++;
            // while (this.deeper_searches.length + 1 >= analyze_depth &&
            //   this.deeper_searches.length > 0
            // ) {
            //   this.deeper_searches.pop();
            // }
            this.deeper_searches.push(data);

          }.bind(this))
    }
  }
}
</script>

<style scoped>
.move {
  display: flex;
}
.rhs {
  text-align: center;
}
.columns {
  display: flex;
  flex-diration: row;
}
</style>