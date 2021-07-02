<template>
  <div>
    <div v-for="position in positions" :key="position.id">
      <div class="move">
        <Chessboard :fen="fen"
                    :move_list="position.move_list"
                    @click.native="$emit('selected', position, analyze_depth)" />
        <MoveWithScore
            :move="position.move"
            :score="position.score" />
      </div>
    </div>
  </div>
</template>

<script>
import Chessboard from "@/components/Chessboard";
import MoveWithScore from "@/components/MoveWithScore";

export default {
  name: "PositionList.vue",
  components: {MoveWithScore, Chessboard},
  props: {
    url: String,
    fen: String,
    analyze_depth: Number,
    positions: Array
  },
  watch: {
    selected_position: function(new_val, old_val) {
      console.log(new_val, old_val);
      let decisionId = encodeURIComponent(this.selected_search.decision_id);
      let url = this.url + '/api/fetch.php?object=positions&decision_id='+ decisionId;
      fetch(url)
          .then(data => data.json())
          .then(data => {
            data.forEach(position => position.move_list = [position.move]);
            this.positions = data;
          })
    }
  }
}
</script>

<style scoped>

</style>